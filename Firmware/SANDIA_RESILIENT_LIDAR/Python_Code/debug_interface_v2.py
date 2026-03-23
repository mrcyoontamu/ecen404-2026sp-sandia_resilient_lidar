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

FRAME_WIDTH = 320
FRAME_HEIGHT = 240
FRAME_SIZE_BYTES = FRAME_WIDTH * FRAME_HEIGHT * 2
NUM_DCS_FRAMES = 4
HEADER = b"\xAA\x55\xAA\x55"

# 24 MHz hardcoded modulation frequency for now.
MODULATION_HZ = 24e6
SPEED_OF_LIGHT_M_PER_S = 299792458.0

# Unambiguous range for iToF phase at modulation frequency f is c/(2f).
MAX_DEPTH_M = SPEED_OF_LIGHT_M_PER_S / (2.0 * MODULATION_HZ)
DEPTH_SCALE_M_PER_RAD = SPEED_OF_LIGHT_M_PER_S / (4.0 * np.pi * MODULATION_HZ)

# Known ST USB IDs and debug-port exclusion.
TARGET_VID = 0x0483
TARGET_PIDS = {0x5740}
EXCLUDED_PIDS = {0x374B, 0x374D, 0x374E, 0x3752, 0x3753}
EXCLUDED_KEYWORDS = ("stlink", "st-link", "debugger")

# Display options.
WINDOW_NAME = "Depth Debug Interface v2"
DISPLAY_SCALE = 3


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


def reconstruct_epc660_rows(raw_frame):
	# Even rows are top-half (reverse), odd rows are bottom-half.
	top_half = raw_frame[0::2][::-1, :]
	bottom_half = raw_frame[1::2, :]
	frame = np.vstack((top_half, bottom_half))
	# Keep vertical orientation consistent with your working script.
	return np.flipud(frame)


def read_one_u16_frame(ser):
	if not find_header(ser):
		return None

	data = ser.read(FRAME_SIZE_BYTES)
	if len(data) != FRAME_SIZE_BYTES:
		print(f"Incomplete frame ({len(data)} bytes)")
		return None

	raw = np.frombuffer(data, dtype="<u2").reshape((FRAME_HEIGHT, FRAME_WIDTH))
	return reconstruct_epc660_rows(raw)


def read_dcs_quad(ser):
	frames = []
	for _ in range(NUM_DCS_FRAMES):
		frame = read_one_u16_frame(ser)
		if frame is None:
			return None
		frames.append(frame)
	return frames


def compute_phase_and_depth(dcs_frames):
	f0, f1, f2, f3 = dcs_frames

	# Mask invalid/saturated pixels first.
	sat_mask = (f0 == 0xFFFF) | (f1 == 0xFFFF) | (f2 == 0xFFFF) | (f3 == 0xFFFF)

	f0f = f0.astype(np.float32)
	f1f = f1.astype(np.float32)
	f2f = f2.astype(np.float32)
	f3f = f3.astype(np.float32)

	# Requested formula: atan((f3-f1)/(f2-f0)) + pi.
	# Use atan2 for stability and correct quadrant behavior.
	num = f3f - f1f
	den = f2f - f0f
	phase_rad = np.arctan2(num, den) + np.pi

	depth_m = phase_rad * DEPTH_SCALE_M_PER_RAD
	depth_m[sat_mask] = np.nan

	return phase_rad, depth_m, sat_mask


def depth_to_colormap(depth_m, sat_mask):
	clipped = np.clip(depth_m, 0.0, MAX_DEPTH_M)
	norm = np.nan_to_num(clipped / MAX_DEPTH_M, nan=0.0)
	gray = (norm * 255.0).astype(np.uint8)
	bgr = cv2.applyColorMap(gray, cv2.COLORMAP_TURBO)
	bgr[sat_mask] = (0, 0, 0)
	return bgr


def save_snapshot(state):
	if state["depth_m"] is None:
		print("No frame available to save yet.")
		return

	os.makedirs("captures", exist_ok=True)
	stamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
	npz_path = os.path.join("captures", f"capture_{stamp}.npz")
	png_path = os.path.join("captures", f"capture_{stamp}.png")

	np.savez_compressed(
		npz_path,
		dcs0=state["dcs_frames"][0],
		dcs1=state["dcs_frames"][1],
		dcs2=state["dcs_frames"][2],
		dcs3=state["dcs_frames"][3],
		phase_rad=state["phase_rad"],
		depth_m=state["depth_m"],
		modulation_hz=MODULATION_HZ,
	)
	cv2.imwrite(png_path, state["display_bgr"])
	print(f"Saved: {npz_path}")
	print(f"Saved: {png_path}")


def on_mouse(event, x, y, flags, state):
	del flags
	if event != cv2.EVENT_LBUTTONDOWN:
		return

	depth_m = state["depth_m"]
	if depth_m is None:
		return

	u = x // DISPLAY_SCALE
	v = y // DISPLAY_SCALE
	if u < 0 or u >= FRAME_WIDTH or v < 0 or v >= FRAME_HEIGHT:
		return

	state["clicked_uv"] = (u, v)
	d = depth_m[v, u]
	p = state["phase_rad"][v, u]
	f0 = int(state["dcs_frames"][0][v, u])
	f1 = int(state["dcs_frames"][1][v, u])
	f2 = int(state["dcs_frames"][2][v, u])
	f3 = int(state["dcs_frames"][3][v, u])

	if np.isnan(d):
		print(f"Click ({u}, {v}) -> invalid/saturated pixel")
	else:
		print(
			f"Click ({u}, {v}) -> depth={d:.4f} m ({d * 3.28084:.3f} ft), "
			f"phase={p:.4f} rad, DCS=[{f0}, {f1}, {f2}, {f3}]"
		)


def draw_overlay(display_bgr, state):
	view = display_bgr.copy()

	status = "PAUSED" if state["paused"] else "LIVE"
	cv2.putText(view, status, (10, 25), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 255), 2)
	cv2.putText(
		view,
		"keys: q quit | space pause | s save",
		(10, 50),
		cv2.FONT_HERSHEY_SIMPLEX,
		0.5,
		(255, 255, 255),
		1,
	)

	if state["clicked_uv"] is not None and state["depth_m"] is not None:
		u, v = state["clicked_uv"]
		x = u * DISPLAY_SCALE
		y = v * DISPLAY_SCALE
		cv2.drawMarker(view, (x, y), (255, 255, 255), cv2.MARKER_CROSS, 12, 1)

		d = state["depth_m"][v, u]
		if np.isnan(d):
			text = f"({u},{v}) invalid"
		else:
			text = f"({u},{v}) {d:.3f} m / {d * 3.28084:.3f} ft"
		cv2.putText(view, text, (10, 75), cv2.FONT_HERSHEY_SIMPLEX, 0.55, (255, 255, 255), 2)

	return view


def main():
	cv2.namedWindow(WINDOW_NAME, cv2.WINDOW_NORMAL)
	cv2.resizeWindow(WINDOW_NAME, FRAME_WIDTH * DISPLAY_SCALE, FRAME_HEIGHT * DISPLAY_SCALE)

	state = {
		"paused": False,
		"clicked_uv": None,
		"dcs_frames": None,
		"phase_rad": None,
		"depth_m": None,
		"display_bgr": None,
	}
	cv2.setMouseCallback(WINDOW_NAME, on_mouse, state)

	ser = None

	try:
		while True:
			if ser is None:
				ser = connect_with_retry(BAUD, TIMEOUT)

			try:
				if not state["paused"]:
					dcs_frames = read_dcs_quad(ser)
					if dcs_frames is None:
						continue

					phase_rad, depth_m, sat_mask = compute_phase_and_depth(dcs_frames)
					display_bgr = depth_to_colormap(depth_m, sat_mask)
					display_bgr = cv2.resize(
						display_bgr,
						(FRAME_WIDTH * DISPLAY_SCALE, FRAME_HEIGHT * DISPLAY_SCALE),
						interpolation=cv2.INTER_NEAREST,
					)

					state["dcs_frames"] = dcs_frames
					state["phase_rad"] = phase_rad
					state["depth_m"] = depth_m
					state["display_bgr"] = display_bgr

				if state["display_bgr"] is not None:
					view = draw_overlay(state["display_bgr"], state)
					cv2.imshow(WINDOW_NAME, view)

				key = cv2.waitKey(30 if state["paused"] else 1) & 0xFF
				if key == ord("q"):
					break
				if key == ord(" "):
					state["paused"] = not state["paused"]
				if key == ord("s"):
					save_snapshot(state)

			except (serial.SerialException, OSError) as exc:
				print(f"Serial link lost: {exc}")
				try:
					ser.close()
				except Exception:
					pass
				ser = None
				print(f"Reconnecting in {RECONNECT_INTERVAL_S} seconds...")
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
