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

# Known ST USB IDs and debug-port exclusion.
TARGET_VID = 0x0483
TARGET_PIDS = {0x5740}
EXCLUDED_PIDS = {0x374B, 0x374D, 0x374E, 0x3752, 0x3753}
EXCLUDED_KEYWORDS = ("stlink", "st-link", "debugger")

# Display options.
WINDOW_NAME = "Intruder Amplitude"
DISPLAY_SCALE = 3
SHOW_FILTER_WORKFLOW = True

# Amplitude-only intruder tuning.
# Low, mid, high bins are selected using the max of current/background amplitude.
AMP_LOW_MAX_LSB = 15.0
AMP_HIGH_MIN_LSB = 50.0

DIFF_THRESHOLD_LOW_LSB = 8
DIFF_THRESHOLD_MID_LSB = 8
DIFF_THRESHOLD_HIGH_LSB = 8

BLOB_TRIGGER_PIXELS = 90
TEMPORAL_TRIGGER_FRAMES = 1
MASK_BLUR_KERNEL = 21
ENABLE_MASK_BLUR = True

# Visualization scaling for amplitude and delta-amplitude.
AUTO_SCALE_AMPLITUDE = True
AMPLITUDE_DISPLAY_MAX_LSB = 1200.0
DIFF_DISPLAY_MAX_LSB = 40.0

CMD_GET_FRAME = b"G"
CMD_STATUS = b"S"


def draw_outlined_text(img, text, org, font_scale=0.50, thickness=1):
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


def compute_amplitude_lsb(dcs_frames):
	f0, f1, f2, f3 = dcs_frames

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
	amplitude_lsb[sat_mask] = np.nan
	return amplitude_lsb, sat_mask


def adaptive_diff_threshold_map_lsb(ref_amp_lsb, cur_amp_lsb):
	amplitude_for_bin = np.nanmax(np.stack((ref_amp_lsb, cur_amp_lsb)), axis=0)

	threshold_map = np.full(amplitude_for_bin.shape, DIFF_THRESHOLD_MID_LSB, dtype=np.float32)
	threshold_map[amplitude_for_bin <= AMP_LOW_MAX_LSB] = DIFF_THRESHOLD_LOW_LSB
	threshold_map[amplitude_for_bin >= AMP_HIGH_MIN_LSB] = DIFF_THRESHOLD_HIGH_LSB
	return threshold_map


def amplitude_to_colormap(amplitude_lsb):
	if AUTO_SCALE_AMPLITUDE:
		valid = amplitude_lsb[np.isfinite(amplitude_lsb)]
		if valid.size > 10:
			clip_max = max(50.0, float(np.percentile(valid, 99.0)))
		else:
			clip_max = AMPLITUDE_DISPLAY_MAX_LSB
	else:
		clip_max = AMPLITUDE_DISPLAY_MAX_LSB

	clipped = np.clip(np.nan_to_num(amplitude_lsb, nan=0.0), 0.0, clip_max)
	norm = clipped / max(clip_max, 1e-6)
	gray = (norm * 255.0).astype(np.uint8)
	bgr = cv2.applyColorMap(gray, cv2.COLORMAP_TURBO)
	bgr[~np.isfinite(amplitude_lsb)] = (0, 0, 0)
	return bgr


def make_mask_bgr(mask):
	if mask is None:
		return np.zeros((FRAME_HEIGHT, FRAME_WIDTH, 3), dtype=np.uint8)
	gray = mask.astype(np.uint8)
	return cv2.cvtColor(gray, cv2.COLOR_GRAY2BGR)


def make_diff_bgr(diff_lsb):
	if diff_lsb is None:
		return np.zeros((FRAME_HEIGHT, FRAME_WIDTH, 3), dtype=np.uint8)
	norm = np.clip(diff_lsb / max(DIFF_DISPLAY_MAX_LSB, 1e-6), 0.0, 1.0)
	gray = (norm * 255.0).astype(np.uint8)
	return cv2.applyColorMap(gray, cv2.COLORMAP_TURBO)


def label_panel(panel_bgr, text):
	draw_outlined_text(panel_bgr, text, (8, 20), font_scale=0.55, thickness=1)
	return panel_bgr


def build_workflow_view(amplitude_bgr, state):
	amp_panel = label_panel(amplitude_bgr.copy(), "Amplitude (Current)")
	bg_panel = label_panel(amplitude_to_colormap(state["ref_amp_lsb"]) if state["ref_amp_lsb"] is not None else np.zeros_like(amplitude_bgr), "Amplitude (Background)")
	diff_panel = label_panel(make_diff_bgr(state["diff_lsb"]), "|Amp - Background|")
	candidate_panel = label_panel(make_mask_bgr(state["candidate_mask"]), "Threshold Candidate")
	blur_panel = label_panel(make_mask_bgr(state["post_blur_mask"]), "After Mask Blur")
	overlay_panel = amplitude_bgr.copy()
	if state["fg_mask"] is not None:
		overlay_panel[state["fg_mask"] > 0] = (0, 0, 255)
	overlay_panel = label_panel(overlay_panel, "Final Overlay")

	row1 = np.hstack((amp_panel, bg_panel, diff_panel))
	row2 = np.hstack((candidate_panel, blur_panel, overlay_panel))
	return np.vstack((row1, row2))


def update_intruder_detection(state, amp_lsb):
	valid_now = np.isfinite(amp_lsb)
	fg_mask = np.zeros(amp_lsb.shape, dtype=np.uint8)
	candidate_mask = np.zeros(amp_lsb.shape, dtype=np.uint8)
	post_blur_mask = np.zeros(amp_lsb.shape, dtype=np.uint8)
	diff_for_display_lsb = None
	trigger_metric = 0.0

	if state["ref_amp_lsb"] is not None:
		ref = state["ref_amp_lsb"]
		valid_ref = np.isfinite(ref)

		diff_lsb = np.abs(amp_lsb - ref)
		diff_for_display_lsb = np.nan_to_num(diff_lsb, nan=0.0, posinf=0.0, neginf=0.0).astype(np.float32)

		threshold_map = adaptive_diff_threshold_map_lsb(ref, amp_lsb)
		candidate = valid_now & valid_ref & (diff_lsb >= threshold_map)
		candidate_mask[candidate] = 255
		fg_mask = candidate_mask.copy()

		blur_kernel = max(0, int(MASK_BLUR_KERNEL))
		if ENABLE_MASK_BLUR and blur_kernel > 0:
			if blur_kernel % 2 == 0:
				blur_kernel += 1
			if blur_kernel > 1:
				fg_mask = cv2.medianBlur(fg_mask, blur_kernel)

		post_blur_mask = fg_mask.copy()
		foreground_pixels = int(np.count_nonzero(fg_mask))
		trigger_metric = float(foreground_pixels)

		if trigger_metric >= BLOB_TRIGGER_PIXELS:
			state["trigger_counter"] += 1
		else:
			state["trigger_counter"] = 0

		if (not state["triggered"]) and state["trigger_counter"] >= TEMPORAL_TRIGGER_FRAMES:
			state["triggered"] = True
			state["trigger_timestamp"] = timestamp_text()
			log_event("INTRUDER DETECTED (amplitude)")
	else:
		foreground_pixels = 0
		state["trigger_counter"] = 0

	state["fg_mask"] = fg_mask
	state["candidate_mask"] = candidate_mask
	state["post_blur_mask"] = post_blur_mask
	state["diff_lsb"] = diff_for_display_lsb
	state["foreground_pixels"] = foreground_pixels
	state["trigger_metric_pixels"] = trigger_metric if state["ref_amp_lsb"] is not None else 0.0


def rearm_detection(state):
	state["triggered"] = False
	state["trigger_counter"] = 0
	state["trigger_timestamp"] = None
	state["ref_amp_lsb"] = None
	state["fg_mask"] = None
	state["candidate_mask"] = None
	state["post_blur_mask"] = None
	state["diff_lsb"] = None
	state["foreground_pixels"] = 0
	state["trigger_metric_pixels"] = 0.0
	log_event("System rearmed. Press 'b' to capture a new amplitude background.")


def capture_background(state):
	if state["amp_lsb"] is None:
		log_event("No frame available yet to capture background.")
		return

	state["ref_amp_lsb"] = state["amp_lsb"].copy()
	state["trigger_counter"] = 0
	state["triggered"] = False
	state["trigger_timestamp"] = None
	log_event("Amplitude background captured")


def on_mouse(event, x, y, flags, state):
	del flags
	if event != cv2.EVENT_LBUTTONDOWN:
		return

	amp_lsb = state["amp_lsb"]
	if amp_lsb is None:
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

	a = amp_lsb[v, u]
	if np.isnan(a):
		print(f"Click ({u}, {v}) -> invalid/saturated pixel")
		return

	if state["ref_amp_lsb"] is not None:
		bg = state["ref_amp_lsb"][v, u]
		d = np.abs(a - bg) if np.isfinite(bg) else np.nan
		if np.isfinite(bg):
			print(f"Click ({u}, {v}) -> amp={a:.2f} bg={bg:.2f} diff={d:.2f} LSB")
		else:
			print(f"Click ({u}, {v}) -> amp={a:.2f} bg=nan")
	else:
		print(f"Click ({u}, {v}) -> amp={a:.2f} LSB")


def draw_overlay(display_bgr, state):
	view = display_bgr.copy()

	base = "PAUSED" if state["paused"] else "LIVE"
	mode = "ARMED" if state["ref_amp_lsb"] is not None else "UNARMED"
	status = f"{base} | {mode}"
	draw_outlined_text(view, status, (10, 25), font_scale=0.7, thickness=2)

	if state["triggered"]:
		cv2.rectangle(view, (0, 0), (view.shape[1], 40), (0, 0, 255), -1)
		cv2.putText(
			view,
			"INTRUDER DETECTED - PRESS 'r' TO REARM",
			(10, 28),
			cv2.FONT_HERSHEY_SIMPLEX,
			0.65,
			(255, 255, 255),
			2,
		)

	draw_outlined_text(
		view,
		"keys: q quit | space pause | s save | b bg capture | r rearm | w workflow",
		(10, 55),
		font_scale=0.48,
		thickness=1,
	)

	draw_outlined_text(
		view,
		f"fg_pixels={state['foreground_pixels']} trigger={state['trigger_metric_pixels']:.0f}/{BLOB_TRIGGER_PIXELS}",
		(10, 75),
		font_scale=0.50,
		thickness=1,
	)

	draw_outlined_text(
		view,
		f"amp bins: <= {AMP_LOW_MAX_LSB:.0f}, mid, >= {AMP_HIGH_MIN_LSB:.0f} | diff thr: {DIFF_THRESHOLD_LOW_LSB:.1f}/{DIFF_THRESHOLD_MID_LSB:.1f}/{DIFF_THRESHOLD_HIGH_LSB:.1f}",
		(10, 95),
		font_scale=0.48,
		thickness=1,
	)

	draw_outlined_text(
		view,
		f"mask_blur={'on' if ENABLE_MASK_BLUR else 'off'}({MASK_BLUR_KERNEL}) temporal={TEMPORAL_TRIGGER_FRAMES}",
		(10, 115),
		font_scale=0.50,
		thickness=1,
	)

	if state["trigger_timestamp"] is not None:
		draw_outlined_text(
			view,
			f"triggered_at={state['trigger_timestamp']}",
			(10, 135),
			font_scale=0.50,
			thickness=1,
		)

	if (not state["show_workflow"]) and state["fg_mask"] is not None:
		mask_small = cv2.resize(
			state["fg_mask"],
			(FRAME_WIDTH * DISPLAY_SCALE, FRAME_HEIGHT * DISPLAY_SCALE),
			interpolation=cv2.INTER_NEAREST,
		)
		view[mask_small > 0] = (0, 0, 255)

	if state["clicked_uv"] is not None and state["amp_lsb"] is not None:
		u, v = state["clicked_uv"]
		a = state["amp_lsb"][v, u]
		if np.isnan(a):
			click_text = f"click ({u},{v}) invalid"
		else:
			if state["ref_amp_lsb"] is not None and np.isfinite(state["ref_amp_lsb"][v, u]):
				bg = state["ref_amp_lsb"][v, u]
				d = abs(a - bg)
				click_text = f"click ({u},{v}) amp={a:.2f} bg={bg:.2f} diff={d:.2f}"
			else:
				click_text = f"click ({u},{v}) amp={a:.2f}"
		draw_outlined_text(view, click_text, (10, 155), font_scale=0.52, thickness=1)

	return view


def save_snapshot(state):
	if state["amp_lsb"] is None:
		print("No frame available to save yet.")
		return

	os.makedirs("captures", exist_ok=True)
	stamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
	npz_path = os.path.join("captures", f"intruder_amp_{stamp}.npz")
	png_path = os.path.join("captures", f"intruder_amp_{stamp}.png")

	np.savez_compressed(
		npz_path,
		dcs0=state["dcs_frames"][0],
		dcs1=state["dcs_frames"][1],
		dcs2=state["dcs_frames"][2],
		dcs3=state["dcs_frames"][3],
		amp_lsb=state["amp_lsb"],
		ref_amp_lsb=state["ref_amp_lsb"],
		fg_mask=state["fg_mask"],
		triggered=state["triggered"],
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
	if state["amp_lsb"] is None:
		return

	amp_bgr = amplitude_to_colormap(state["amp_lsb"])
	if state["show_workflow"]:
		state["display_bgr"] = build_workflow_view(amp_bgr, state)
	else:
		state["display_bgr"] = cv2.resize(
			amp_bgr,
			(FRAME_WIDTH * DISPLAY_SCALE, FRAME_HEIGHT * DISPLAY_SCALE),
			interpolation=cv2.INTER_NEAREST,
		)


def main():
	cv2.namedWindow(WINDOW_NAME, cv2.WINDOW_NORMAL)
	resize_for_view_mode(SHOW_FILTER_WORKFLOW)

	state = {
		"paused": False,
		"show_workflow": SHOW_FILTER_WORKFLOW,
		"clicked_uv": None,
		"dcs_frames": None,
		"amp_lsb": None,
		"display_bgr": None,
		"modulation_hz": MODULATION_HZ,
		"ref_amp_lsb": None,
		"fg_mask": None,
		"candidate_mask": None,
		"post_blur_mask": None,
		"diff_lsb": None,
		"foreground_pixels": 0,
		"trigger_metric_pixels": 0.0,
		"trigger_counter": 0,
		"triggered": False,
		"trigger_timestamp": None,
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

					amp_lsb, sat_mask = compute_amplitude_lsb(dcs_frames)
					update_intruder_detection(state, amp_lsb)

					amp_bgr = amplitude_to_colormap(amp_lsb)
					if state["show_workflow"]:
						display_bgr = build_workflow_view(amp_bgr, state)
					else:
						display_bgr = cv2.resize(
							amp_bgr,
							(FRAME_WIDTH * DISPLAY_SCALE, FRAME_HEIGHT * DISPLAY_SCALE),
							interpolation=cv2.INTER_NEAREST,
						)

					state["dcs_frames"] = dcs_frames
					state["amp_lsb"] = amp_lsb
					state["display_bgr"] = display_bgr

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
				if key == ord("b"):
					capture_background(state)
				if key == ord("r"):
					rearm_detection(state)
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
