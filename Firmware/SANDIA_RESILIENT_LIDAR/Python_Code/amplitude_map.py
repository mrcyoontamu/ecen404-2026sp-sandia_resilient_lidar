import os
import time
from datetime import datetime

import cv2
import numpy as np
import serial
from serial.tools import list_ports


# --- CONFIGURATION ---
# Set FORCE_PORT to a COM string (e.g., 'COM7') to bypass auto-discovery.
FORCE_PORT = None

BAUD = 921600
TIMEOUT = 5
RECONNECT_INTERVAL_S = 5
STATUS_POLL_INTERVAL_S = 1.0

FRAME_WIDTH = 320
FRAME_HEIGHT = 240
FRAME_SIZE_BYTES = FRAME_WIDTH * FRAME_HEIGHT * 2
NUM_DCS_FRAMES = 4
QUAD_SIZE_BYTES = FRAME_SIZE_BYTES * NUM_DCS_FRAMES
HEADER = b"\xAA\x55\xAA\x55"

# 24 MHz hardcoded modulation frequency for now.
DEFAULT_MODULATION_HZ = 24e6
MODULATION_HZ = DEFAULT_MODULATION_HZ
SPEED_OF_LIGHT_M_PER_S = 299792458.0

# Datasheet guidance: low amplitude means weak phase confidence.
AMPLITUDE_MIN_LSB = 25.0

# Known ST USB IDs and debug-port exclusion.
TARGET_VID = 0x0483
TARGET_PIDS = {0x5740}
EXCLUDED_PIDS = {0x374B, 0x374D, 0x374E, 0x3752, 0x3753}
EXCLUDED_KEYWORDS = ("stlink", "st-link", "debugger")

# Display options.
WINDOW_NAME = "Amplitude Map"
DISPLAY_SCALE = 3
SHOW_RAW_WORKFLOW = True

# Color mapping controls.
AUTO_SCALE_AMPLITUDE = True
AMPLITUDE_DISPLAY_MAX_LSB = 1200.0
RAW_DISPLAY_MIN = 0.0
RAW_DISPLAY_MAX = 4095.0

CMD_GET_FRAME = b"G"
CMD_STATUS = b"S"


def draw_outlined_text(img, text, org, font_scale=0.55, thickness=1):
	# Draw black stroke first, then white text for readability on bright/dark pixels.
	cv2.putText(
		img,
		text,
		org,
		cv2.FONT_HERSHEY_SIMPLEX,
		font_scale,
		(0, 0, 0),
		thickness + 2,
		cv2.LINE_AA,
	)
	cv2.putText(
		img,
		text,
		org,
		cv2.FONT_HERSHEY_SIMPLEX,
		font_scale,
		(255, 255, 255),
		thickness,
		cv2.LINE_AA,
	)


def list_available_ports():
	ports = sorted([p.device for p in list_ports.comports()])
	return ", ".join(ports) if ports else "none"


def pick_target_port():
	ports = list(list_ports.comports())

	if FORCE_PORT:
		return FORCE_PORT

	def is_excluded(port_info):
		desc = (port_info.description or "").lower()
		manu = (port_info.manufacturer or "").lower()
		hwid = (port_info.hwid or "").lower()

		if port_info.pid in EXCLUDED_PIDS:
			return True

		for token in EXCLUDED_KEYWORDS:
			if token in desc or token in manu or token in hwid:
				return True

		return False

	# 1) Strong match: exact target VID/PID.
	for p in ports:
		if is_excluded(p):
			continue
		if p.vid == TARGET_VID and p.pid in TARGET_PIDS:
			return p.device

	# 2) Soft match fallback.
	for p in ports:
		if is_excluded(p):
			continue

		desc = (p.description or "").lower()
		manu = (p.manufacturer or "").lower()
		hwid = (p.hwid or "").lower()
		if (
			"cdc" in desc
			or "usb serial" in desc
			or "virtual com" in desc
			or "communications port" in desc
			or "vid:pid=0483" in hwid
			or "usb\\class_02" in hwid
			or "usb" in manu
		):
			return p.device

	return None


def connect_with_retry(baud, timeout):
	while True:
		port = pick_target_port()
		if port is None:
			print("No matching serial device found.")
			print(f"Available ports: {list_available_ports()}")
			print(f"Retrying in {RECONNECT_INTERVAL_S} seconds... (Ctrl+C to quit)")
			time.sleep(RECONNECT_INTERVAL_S)
			continue

		try:
			ser = serial.Serial(port, baud, timeout=timeout)
			print(f"Connected to {port}")
			flush_serial_input(ser)
			return ser
		except serial.SerialException as exc:
			print(f"Connect failed for {port}: {exc}")
			print(f"Available ports: {list_available_ports()}")
			print(f"Retrying in {RECONNECT_INTERVAL_S} seconds... (Ctrl+C to quit)")
			time.sleep(RECONNECT_INTERVAL_S)


def find_header(ser):
	buffer = b""
	while True:
		byte = ser.read(1)
		if not byte:
			return False
		buffer += byte
		if len(buffer) > 4:
			buffer = buffer[-4:]
		if buffer == HEADER:
			return True


def timestamp_text():
	return datetime.now().strftime("%H:%M:%S.%f")[:-3]


def log_event(message):
	print(f"[{timestamp_text()}] {message}")


def flush_serial_input(ser):
	try:
		ser.reset_input_buffer()
		log_event("RX buffer flushed")
	except Exception as exc:
		log_event(f"RX flush failed: {exc}")


def send_command(ser, cmd):
	ser.write(cmd)
	ser.flush()


def modulation_hz_from_divider(mod_div):
	# Datasheet table: mod_clk = 96MHz/(divider+1), LED modulation fLED = mod_clk/4.
	if mod_div < 0:
		return None
	return 96e6 / float(mod_div + 1) / 4.0


def parse_status_fields(status_line):
	fields = {}
	for token in status_line.split():
		if "=" not in token:
			continue
		key, value = token.split("=", 1)
		fields[key] = value
	return fields


def read_status_line(ser, timeout_s=0.20):
	deadline = time.monotonic() + timeout_s
	buffer = bytearray()

	while time.monotonic() < deadline:
		ch = ser.read(1)
		if not ch:
			continue
		if ch == b"\n":
			break
		if ch != b"\r":
			buffer.extend(ch)

	if not buffer:
		return None

	try:
		return buffer.decode("ascii", errors="replace")
	except Exception:
		return None


def reconstruct_epc660_rows(raw_frame):
	# Even rows are top-half (reverse), odd rows are bottom-half.
	top_half = raw_frame[0::2][::-1, :]
	bottom_half = raw_frame[1::2, :]
	frame = np.vstack((top_half, bottom_half))
	# Keep vertical orientation consistent with other visualization scripts.
	return np.flipud(frame)


def read_dcs_quad(ser):
	if not find_header(ser):
		return None

	data = ser.read(QUAD_SIZE_BYTES)
	if len(data) != QUAD_SIZE_BYTES:
		print(f"Incomplete quad ({len(data)} bytes)")
		return None

	frames = []
	for i in range(NUM_DCS_FRAMES):
		start = i * FRAME_SIZE_BYTES
		end = start + FRAME_SIZE_BYTES
		raw = np.frombuffer(data[start:end], dtype="<u2").reshape((FRAME_HEIGHT, FRAME_WIDTH))
		frames.append(reconstruct_epc660_rows(raw))

	return frames


def request_one_dcs_quad(ser):
	send_command(ser, CMD_GET_FRAME)
	return read_dcs_quad(ser)


def compute_raw_and_amplitude(dcs_frames):
	f0, f1, f2, f3 = dcs_frames

	# Raw sensor values are 12-bit with 0x0FFF saturation code.
	raw0 = f0 & 0x0FFF
	raw1 = f1 & 0x0FFF
	raw2 = f2 & 0x0FFF
	raw3 = f3 & 0x0FFF
	sat_mask = (raw0 == 0x0FFF) | (raw1 == 0x0FFF) | (raw2 == 0x0FFF) | (raw3 == 0x0FFF)

	i1 = raw0.astype(np.float32) - 2048.0
	i2 = raw1.astype(np.float32) - 2048.0
	i3 = raw2.astype(np.float32) - 2048.0
	i4 = raw3.astype(np.float32) - 2048.0

	q = i4 - i2
	in_phase = i3 - i1
	amplitude_lsb = 0.5 * np.sqrt(q * q + in_phase * in_phase)
	low_signal_mask = amplitude_lsb < AMPLITUDE_MIN_LSB

	return (raw0, raw1, raw2, raw3), amplitude_lsb, sat_mask, low_signal_mask


def scalar_to_colormap(data, clip_min, clip_max, invalid_mask=None):
	if clip_max <= clip_min:
		clip_max = clip_min + 1.0
	clipped = np.clip(data, clip_min, clip_max)
	norm = (clipped - clip_min) / (clip_max - clip_min)
	gray = (norm * 255.0).astype(np.uint8)
	bgr = cv2.applyColorMap(gray, cv2.COLORMAP_TURBO)
	if invalid_mask is not None:
		bgr[invalid_mask] = (0, 0, 0)
	return bgr


def dcs_to_colormap(raw_frame):
	return scalar_to_colormap(raw_frame.astype(np.float32), RAW_DISPLAY_MIN, RAW_DISPLAY_MAX)


def amplitude_to_colormap(amplitude_lsb, invalid_mask):
	if AUTO_SCALE_AMPLITUDE:
		valid = amplitude_lsb[~invalid_mask]
		if valid.size > 10:
			clip_max = max(50.0, float(np.percentile(valid, 99.0)))
		else:
			clip_max = AMPLITUDE_DISPLAY_MAX_LSB
	else:
		clip_max = AMPLITUDE_DISPLAY_MAX_LSB

	return scalar_to_colormap(amplitude_lsb, 0.0, clip_max, invalid_mask=invalid_mask)


def make_mask_bgr(mask):
	gray = (mask.astype(np.uint8) * 255) if mask.dtype != np.uint8 else mask
	return cv2.cvtColor(gray, cv2.COLOR_GRAY2BGR)


def label_panel(panel_bgr, text):
	draw_outlined_text(panel_bgr, text, (8, 20), font_scale=0.55, thickness=1)
	return panel_bgr


def build_workflow_view(raw_frames, amplitude_lsb, sat_mask, low_signal_mask):
	dcs0_panel = label_panel(dcs_to_colormap(raw_frames[0]), "DCS0 (12-bit Raw)")
	dcs1_panel = label_panel(dcs_to_colormap(raw_frames[1]), "DCS1 (12-bit Raw)")
	dcs2_panel = label_panel(dcs_to_colormap(raw_frames[2]), "DCS2 (12-bit Raw)")
	dcs3_panel = label_panel(dcs_to_colormap(raw_frames[3]), "DCS3 (12-bit Raw)")
	amp_panel = label_panel(amplitude_to_colormap(amplitude_lsb, sat_mask), "Amplitude (LSB)")
	mask_panel = label_panel(make_mask_bgr(sat_mask | low_signal_mask), "Invalid (sat|low amp)")

	row1 = np.hstack((dcs0_panel, dcs1_panel, dcs2_panel))
	row2 = np.hstack((dcs3_panel, amp_panel, mask_panel))
	return np.vstack((row1, row2))


def save_snapshot(state):
	if state["amplitude_lsb"] is None:
		print("No frame available to save yet.")
		return

	os.makedirs("captures", exist_ok=True)
	stamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
	npz_path = os.path.join("captures", f"amplitude_{stamp}.npz")
	png_path = os.path.join("captures", f"amplitude_{stamp}.png")

	np.savez_compressed(
		npz_path,
		raw0=state["raw_frames"][0],
		raw1=state["raw_frames"][1],
		raw2=state["raw_frames"][2],
		raw3=state["raw_frames"][3],
		amplitude_lsb=state["amplitude_lsb"],
		sat_mask=state["sat_mask"],
		low_signal_mask=state["low_signal_mask"],
		modulation_hz=state["modulation_hz"],
	)

	if state["display_bgr"] is not None:
		overlay = draw_overlay(state["display_bgr"], state)
		cv2.imwrite(png_path, overlay)

	print(f"Saved: {npz_path}")
	print(f"Saved: {png_path}")


def resize_for_view_mode(show_workflow):
	if show_workflow:
		cv2.resizeWindow(WINDOW_NAME, FRAME_WIDTH * 3, FRAME_HEIGHT * 2)
	else:
		cv2.resizeWindow(
			WINDOW_NAME,
			FRAME_WIDTH * DISPLAY_SCALE,
			FRAME_HEIGHT * DISPLAY_SCALE,
		)


def rebuild_display_from_state(state):
	if state["amplitude_lsb"] is None:
		return

	if state["show_workflow"]:
		state["display_bgr"] = build_workflow_view(
			state["raw_frames"],
			state["amplitude_lsb"],
			state["sat_mask"],
			state["low_signal_mask"],
		)
	else:
		amp_bgr = amplitude_to_colormap(state["amplitude_lsb"], state["sat_mask"])
		state["display_bgr"] = cv2.resize(
			amp_bgr,
			(FRAME_WIDTH * DISPLAY_SCALE, FRAME_HEIGHT * DISPLAY_SCALE),
			interpolation=cv2.INTER_NEAREST,
		)


def on_mouse(event, x, y, flags, state):
	del flags
	if event != cv2.EVENT_LBUTTONDOWN:
		return

	if state["raw_frames"] is None:
		return

	if state["show_workflow"]:
		if x < 0 or x >= (FRAME_WIDTH * 3) or y < 0 or y >= (FRAME_HEIGHT * 2):
			return
		u = x % FRAME_WIDTH
		v = y % FRAME_HEIGHT
	else:
		u = x // DISPLAY_SCALE
		v = y // DISPLAY_SCALE
		if u < 0 or u >= FRAME_WIDTH or v < 0 or v >= FRAME_HEIGHT:
			return

	state["clicked_uv"] = (u, v)
	f0 = int(state["raw_frames"][0][v, u])
	f1 = int(state["raw_frames"][1][v, u])
	f2 = int(state["raw_frames"][2][v, u])
	f3 = int(state["raw_frames"][3][v, u])
	amp = float(state["amplitude_lsb"][v, u])
	sat = bool(state["sat_mask"][v, u])
	low = bool(state["low_signal_mask"][v, u])

	print(
		f"Click ({u}, {v}) -> DCS=[{f0}, {f1}, {f2}, {f3}] "
		f"amp={amp:.2f} LSB sat={sat} low_amp={low}"
	)


def draw_overlay(display_bgr, state):
	view = display_bgr.copy()

	status = "PAUSED" if state["paused"] else "LIVE"
	mode = "WORKFLOW" if state["show_workflow"] else "AMPLITUDE"
	draw_outlined_text(view, f"{status} | {mode} | FPS {state['fps']:.1f}", (10, 24), font_scale=0.7, thickness=2)
	draw_outlined_text(
		view,
		f"mod {state['modulation_hz'] / 1e6:.2f} MHz | amp_min {AMPLITUDE_MIN_LSB:.1f} LSB",
		(10, 49),
		font_scale=0.52,
		thickness=1,
	)
	draw_outlined_text(view, "keys: q quit | space pause | s save | w workflow", (10, 72), font_scale=0.5, thickness=1)

	if state["amplitude_lsb"] is not None:
		draw_outlined_text(
			view,
			f"sat_pixels={state['sat_pixels']} low_amp_pixels={state['low_signal_pixels']}",
			(10, 96),
			font_scale=0.52,
			thickness=1,
		)

	if state["clicked_uv"] is not None and state["raw_frames"] is not None:
		u, v = state["clicked_uv"]
		if not state["show_workflow"]:
			x = u * DISPLAY_SCALE
			y = v * DISPLAY_SCALE
			cv2.drawMarker(view, (x, y), (255, 255, 255), cv2.MARKER_CROSS, 12, 1)

		f0 = int(state["raw_frames"][0][v, u])
		f1 = int(state["raw_frames"][1][v, u])
		f2 = int(state["raw_frames"][2][v, u])
		f3 = int(state["raw_frames"][3][v, u])
		amp = float(state["amplitude_lsb"][v, u])
		text = f"({u},{v}) DCS[{f0},{f1},{f2},{f3}] amp={amp:.2f}"
		draw_outlined_text(view, text, (10, 120), font_scale=0.52, thickness=1)

	return view


def main():
	cv2.namedWindow(WINDOW_NAME, cv2.WINDOW_NORMAL)
	resize_for_view_mode(SHOW_RAW_WORKFLOW)

	state = {
		"paused": False,
		"show_workflow": SHOW_RAW_WORKFLOW,
		"clicked_uv": None,
		"raw_frames": None,
		"amplitude_lsb": None,
		"sat_mask": None,
		"low_signal_mask": None,
		"sat_pixels": 0,
		"low_signal_pixels": 0,
		"display_bgr": None,
		"modulation_hz": MODULATION_HZ,
		"fps": 0.0,
		"fps_frame_counter": 0,
		"fps_last_t": time.monotonic(),
	}
	cv2.setMouseCallback(WINDOW_NAME, on_mouse, state)

	ser = None
	next_status_poll = 0.0

	try:
		while True:
			if ser is None:
				ser = connect_with_retry(BAUD, TIMEOUT)
				next_status_poll = time.monotonic()
				log_event("Capture loop armed")

			try:
				now = time.monotonic()
				if now >= next_status_poll:
					send_command(ser, CMD_STATUS)
					status_line = read_status_line(ser, timeout_s=0.25)
					if status_line is not None and status_line.startswith("ST "):
						fields = parse_status_fields(status_line)
						md_str = fields.get("md")
						if md_str is not None:
							try:
								md = int(md_str, 10)
							except ValueError:
								md = -1
							new_mod_hz = modulation_hz_from_divider(md)
							if new_mod_hz is not None and abs(new_mod_hz - state["modulation_hz"]) > 1.0:
								state["modulation_hz"] = new_mod_hz
								log_event(f"Updated modulation from status: md={md} fLED={new_mod_hz / 1e6:.3f}MHz")
						log_event(status_line)
					next_status_poll = now + STATUS_POLL_INTERVAL_S

				if not state["paused"]:
					dcs_frames = request_one_dcs_quad(ser)
					if dcs_frames is None:
						log_event("Frame request timed out or incomplete")
						continue

					raw_frames, amplitude_lsb, sat_mask, low_signal_mask = compute_raw_and_amplitude(dcs_frames)
					invalid_mask = sat_mask | low_signal_mask

					if state["show_workflow"]:
						display_bgr = build_workflow_view(raw_frames, amplitude_lsb, sat_mask, low_signal_mask)
					else:
						amp_bgr = amplitude_to_colormap(amplitude_lsb, sat_mask)
						display_bgr = cv2.resize(
							amp_bgr,
							(FRAME_WIDTH * DISPLAY_SCALE, FRAME_HEIGHT * DISPLAY_SCALE),
							interpolation=cv2.INTER_NEAREST,
						)

					state["raw_frames"] = raw_frames
					state["amplitude_lsb"] = amplitude_lsb
					state["sat_mask"] = sat_mask
					state["low_signal_mask"] = low_signal_mask
					state["sat_pixels"] = int(np.count_nonzero(sat_mask))
					state["low_signal_pixels"] = int(np.count_nonzero(low_signal_mask))
					state["display_bgr"] = display_bgr

					state["fps_frame_counter"] += 1
					now_fps = time.monotonic()
					dt = now_fps - state["fps_last_t"]
					if dt >= 0.5:
						state["fps"] = state["fps_frame_counter"] / dt
						state["fps_frame_counter"] = 0
						state["fps_last_t"] = now_fps

				if state["display_bgr"] is not None:
					view = draw_overlay(state["display_bgr"], state)
					cv2.imshow(WINDOW_NAME, view)

				key = cv2.waitKey(30 if state["paused"] else 1) & 0xFF
				if key == ord("q"):
					break
				if key == ord(" "):
					state["paused"] = not state["paused"]
					flush_serial_input(ser)
					next_status_poll = time.monotonic()
					log_event("PAUSE" if state["paused"] else "RESUME")
				if key == ord("s"):
					save_snapshot(state)
				if key == ord("w"):
					state["show_workflow"] = not state["show_workflow"]
					resize_for_view_mode(state["show_workflow"])
					rebuild_display_from_state(state)
					log_event("Workflow view ON" if state["show_workflow"] else "Workflow view OFF")

			except (serial.SerialException, OSError) as exc:
				log_event(f"Serial link lost: {exc}")
				try:
					ser.close()
				except Exception:
					pass
				ser = None
				log_event(f"Reconnecting in {RECONNECT_INTERVAL_S} seconds...")
				time.sleep(RECONNECT_INTERVAL_S)

	except KeyboardInterrupt:
		print("\nStopped by user.")
	finally:
		if ser is not None:
			try:
				ser.close()
			except Exception:
				pass
		cv2.destroyAllWindows()


if __name__ == "__main__":
	main()
