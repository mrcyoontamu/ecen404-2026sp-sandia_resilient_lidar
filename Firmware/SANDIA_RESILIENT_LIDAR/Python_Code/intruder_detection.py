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

# Datasheet 9.2.2 guidance: amplitudes below this LSB threshold are not useful for distance.
AMPLITUDE_MIN_LSB = 35.0

# Distance offset (DOFFSET in datasheet equation [2]). Keep 0 until calibrated.
DISTANCE_OFFSET_M = -1.8288

# For calibration, keep negative-offset results clamped at 0 m rather than wrapped
# to the unambiguous max range.
WRAP_WITH_UNAMBIGUOUS_RANGE = False

# Unambiguous range for iToF phase at modulation frequency f is c/(2f).
MAX_DEPTH_M = SPEED_OF_LIGHT_M_PER_S / (2.0 * MODULATION_HZ)
DEPTH_SCALE_M_PER_RAD = SPEED_OF_LIGHT_M_PER_S / (4.0 * np.pi * MODULATION_HZ)

# Known ST USB IDs and debug-port exclusion.
TARGET_VID = 0x0483
TARGET_PIDS = {0x5740}
EXCLUDED_PIDS = {0x374B, 0x374D, 0x374E, 0x3752, 0x3753}
EXCLUDED_KEYWORDS = ("stlink", "st-link", "debugger")

# Display options.
WINDOW_NAME = "Intruder Detection"
DISPLAY_SCALE = 3
SHOW_FILTER_WORKFLOW = True

# Intruder algorithm tuning.
DIFF_THRESHOLD_M = 0.1
MIN_BLOB_AREA_PIXELS = 80
BLOB_TRIGGER_PIXELS = 120
TEMPORAL_TRIGGER_FRAMES = 1
MASK_BLUR_KERNEL = 11
ENABLE_MASK_BLUR = True
ENABLE_DEPTH_PRE_FILTER = True
DEPTH_FILTER_KERNEL = 11

# Kalman smoothing for largest contiguous blob area (pixels).
ENABLE_KALMAN = True
KALMAN_USE_FOR_TRIGGER = True
KALMAN_MAX_MISSED_FRAMES = 8
KALMAN_PROCESS_NOISE = 8
KALMAN_MEASUREMENT_NOISE = 300

CMD_GET_FRAME = b"G"
CMD_STATUS = b"S"


def create_blob_area_kalman(initial_area_pixels):
	kf = cv2.KalmanFilter(2, 1)
	kf.transitionMatrix = np.array([[1.0, 1.0], [0.0, 1.0]], dtype=np.float32)
	kf.measurementMatrix = np.array([[1.0, 0.0]], dtype=np.float32)
	kf.processNoiseCov = np.array(
		[[KALMAN_PROCESS_NOISE, 0.0], [0.0, KALMAN_PROCESS_NOISE]],
		dtype=np.float32,
	)
	kf.measurementNoiseCov = np.array([[KALMAN_MEASUREMENT_NOISE]], dtype=np.float32)
	kf.errorCovPost = np.eye(2, dtype=np.float32) * 1.0
	kf.statePre = np.array([[float(initial_area_pixels)], [0.0]], dtype=np.float32)
	kf.statePost = np.array([[float(initial_area_pixels)], [0.0]], dtype=np.float32)
	return kf


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


def compute_depth_params(modulation_hz):
	max_depth_m = SPEED_OF_LIGHT_M_PER_S / (2.0 * modulation_hz)
	depth_scale_m_per_rad = SPEED_OF_LIGHT_M_PER_S / (4.0 * np.pi * modulation_hz)
	return max_depth_m, depth_scale_m_per_rad


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


def compute_phase_and_depth(dcs_frames, depth_scale_m_per_rad, max_depth_m):
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
	low_signal_mask = amplitude_lsb < AMPLITUDE_MIN_LSB

	phase_rad = np.arctan2(q, in_phase)
	phase_rad = np.where(phase_rad < 0.0, phase_rad + 2.0 * np.pi, phase_rad)

	depth_m = phase_rad * depth_scale_m_per_rad + DISTANCE_OFFSET_M
	if WRAP_WITH_UNAMBIGUOUS_RANGE:
		depth_m = np.where(depth_m > max_depth_m, depth_m - max_depth_m, depth_m)
		depth_m = np.where(depth_m < 0.0, depth_m + max_depth_m, depth_m)
	else:
		depth_m = np.clip(depth_m, 0.0, max_depth_m)

	invalid_mask = sat_mask | low_signal_mask
	depth_m[invalid_mask] = np.nan

	return phase_rad, depth_m, invalid_mask


def depth_to_colormap(depth_m, invalid_mask, max_depth_m):
	clipped = np.clip(depth_m, 0.0, max_depth_m)
	norm = np.nan_to_num(clipped / max_depth_m, nan=0.0)
	gray = (norm * 255.0).astype(np.uint8)
	bgr = cv2.applyColorMap(gray, cv2.COLORMAP_TURBO)
	bgr[invalid_mask] = (0, 0, 0)
	return bgr


def filter_depth_for_detection(depth_m):
	filtered = depth_m.copy()
	if not ENABLE_DEPTH_PRE_FILTER:
		return filtered

	kernel_size = max(0, int(DEPTH_FILTER_KERNEL))
	if kernel_size <= 1:
		return filtered
	if kernel_size % 2 == 0:
		kernel_size += 1

	valid_mask = np.isfinite(filtered)
	depth_f32 = np.nan_to_num(filtered, nan=0.0, posinf=0.0, neginf=0.0).astype(np.float32)
	weights = valid_mask.astype(np.float32)
	kernel = np.ones((kernel_size, kernel_size), dtype=np.float32)

	depth_sum = cv2.filter2D(depth_f32, -1, kernel, borderType=cv2.BORDER_REFLECT)
	weight_sum = cv2.filter2D(weights, -1, kernel, borderType=cv2.BORDER_REFLECT)

	blurred = depth_f32.copy()
	has_support = weight_sum > 0.0
	blurred[has_support] = depth_sum[has_support] / weight_sum[has_support]
	blurred[~valid_mask] = np.nan
	return blurred


def update_intruder_detection(state, depth_m, invalid_mask):
	depth_for_model = filter_depth_for_detection(depth_m)
	depth_for_model[invalid_mask] = np.nan
	state["depth_filtered_m"] = depth_for_model.copy()

	valid_mask = np.isfinite(depth_for_model)
	fg_mask = np.zeros(depth_for_model.shape, dtype=np.uint8)
	candidate_mask = np.zeros(depth_for_model.shape, dtype=np.uint8)
	post_blur_mask = np.zeros(depth_for_model.shape, dtype=np.uint8)
	diff_for_display_m = None
	trigger_metric = 0.0

	if state["ref_depth_m"] is not None:
		ref = state["ref_depth_m"]
		diff = np.abs(depth_for_model - ref)
		diff_for_display_m = np.nan_to_num(diff, nan=0.0, posinf=0.0, neginf=0.0).astype(np.float32)

		# Treat validity transitions as foreground events.
		bg_valid_mask = np.isfinite(ref)
		bg_invalid_now_valid = (~bg_valid_mask) & valid_mask
		now_invalid_bg_valid = bg_valid_mask & (~valid_mask)
		candidate = (valid_mask & bg_valid_mask & (diff >= DIFF_THRESHOLD_M)) | bg_invalid_now_valid | now_invalid_bg_valid
		candidate_mask[candidate] = 255
		fg_mask = candidate_mask.copy()

		blur_kernel = max(0, int(MASK_BLUR_KERNEL))
		if ENABLE_MASK_BLUR and blur_kernel > 0:
			if blur_kernel % 2 == 0:
				blur_kernel += 1
			if blur_kernel > 1:
				fg_mask = cv2.medianBlur(fg_mask, blur_kernel)

		post_blur_mask = fg_mask.copy()

		# Keep only contiguous blobs above the minimum area so scattered noise
		# in distant regions cannot add up to a trigger.
		num_labels, labels, stats, _ = cv2.connectedComponentsWithStats(fg_mask, connectivity=8)
		filtered_mask = np.zeros_like(fg_mask)
		largest_blob_pixels = 0
		for label_idx in range(1, num_labels):
			blob_area = int(stats[label_idx, cv2.CC_STAT_AREA])
			if blob_area >= MIN_BLOB_AREA_PIXELS:
				filtered_mask[labels == label_idx] = 255
				if blob_area > largest_blob_pixels:
					largest_blob_pixels = blob_area

		fg_mask = filtered_mask
		foreground_pixels = int(np.count_nonzero(fg_mask))

		if ENABLE_KALMAN:
			if largest_blob_pixels > 0:
				if state["blob_area_kf"] is None:
					state["blob_area_kf"] = create_blob_area_kalman(largest_blob_pixels)
				state["blob_area_kf"].predict()
				measurement = np.array([[float(largest_blob_pixels)]], dtype=np.float32)
				corrected = state["blob_area_kf"].correct(measurement)
				state["smoothed_blob_pixels"] = max(0.0, float(corrected[0, 0]))
				state["blob_missed_frames"] = 0
			elif state["blob_area_kf"] is not None:
				predicted = state["blob_area_kf"].predict()
				state["smoothed_blob_pixels"] = max(0.0, float(predicted[0, 0]))
				state["blob_missed_frames"] += 1
				if state["blob_missed_frames"] > KALMAN_MAX_MISSED_FRAMES:
					state["blob_area_kf"] = None
					state["smoothed_blob_pixels"] = 0.0
					state["blob_missed_frames"] = 0
			else:
				state["smoothed_blob_pixels"] = 0.0
		else:
			state["blob_area_kf"] = None
			state["blob_missed_frames"] = 0
			state["smoothed_blob_pixels"] = float(largest_blob_pixels)

		trigger_metric = state["smoothed_blob_pixels"] if (ENABLE_KALMAN and KALMAN_USE_FOR_TRIGGER) else float(largest_blob_pixels)

		if trigger_metric >= BLOB_TRIGGER_PIXELS:
			state["trigger_counter"] += 1
		else:
			state["trigger_counter"] = 0

		if (not state["triggered"]) and state["trigger_counter"] >= TEMPORAL_TRIGGER_FRAMES:
			state["triggered"] = True
			state["trigger_timestamp"] = timestamp_text()
			log_event("INTRUDER DETECTED")
	else:
		foreground_pixels = 0
		largest_blob_pixels = 0
		state["trigger_counter"] = 0
		state["blob_area_kf"] = None
		state["blob_missed_frames"] = 0
		state["smoothed_blob_pixels"] = 0.0

	state["fg_mask"] = fg_mask
	state["candidate_mask"] = candidate_mask
	state["post_blur_mask"] = post_blur_mask
	state["diff_m"] = diff_for_display_m
	state["foreground_pixels"] = foreground_pixels
	state["largest_blob_pixels"] = largest_blob_pixels
	state["trigger_metric_pixels"] = trigger_metric if state["ref_depth_m"] is not None else 0.0


def rearm_detection(state):
	state["triggered"] = False
	state["trigger_counter"] = 0
	state["trigger_timestamp"] = None
	state["ref_depth_m"] = None
	state["fg_mask"] = None
	state["candidate_mask"] = None
	state["post_blur_mask"] = None
	state["diff_m"] = None
	state["foreground_pixels"] = 0
	state["largest_blob_pixels"] = 0
	state["smoothed_blob_pixels"] = 0.0
	state["trigger_metric_pixels"] = 0.0
	state["blob_area_kf"] = None
	state["blob_missed_frames"] = 0
	log_event("System rearmed. Press 'b' to capture a new background.")


def capture_background(state):
	if state["depth_m"] is None:
		log_event("No frame available yet to capture background.")
		return

	if state["depth_filtered_m"] is not None:
		state["ref_depth_m"] = state["depth_filtered_m"].copy()
	else:
		state["ref_depth_m"] = filter_depth_for_detection(state["depth_m"])
	state["trigger_counter"] = 0
	state["triggered"] = False
	state["trigger_timestamp"] = None
	log_event("Background captured (filtered depth reference)")


def make_mask_bgr(mask):
	if mask is None:
		return np.zeros((FRAME_HEIGHT, FRAME_WIDTH, 3), dtype=np.uint8)
	gray = mask.astype(np.uint8)
	return cv2.cvtColor(gray, cv2.COLOR_GRAY2BGR)


def make_diff_bgr(diff_m):
	if diff_m is None:
		return np.zeros((FRAME_HEIGHT, FRAME_WIDTH, 3), dtype=np.uint8)
	norm = np.clip(diff_m / max(DIFF_THRESHOLD_M * 3.0, 1e-6), 0.0, 1.0)
	gray = (norm * 255.0).astype(np.uint8)
	return cv2.applyColorMap(gray, cv2.COLORMAP_TURBO)


def label_panel(panel_bgr, text):
	draw_outlined_text(panel_bgr, text, (8, 20), font_scale=0.55, thickness=1)
	return panel_bgr


def build_workflow_view(depth_bgr, state):
	depth_panel = label_panel(depth_bgr.copy(), "Depth")
	diff_panel = label_panel(make_diff_bgr(state["diff_m"]), "|Depth - Background|")
	candidate_panel = label_panel(make_mask_bgr(state["candidate_mask"]), "Threshold Candidate")
	blur_panel = label_panel(make_mask_bgr(state["post_blur_mask"]), "After Mask Blur")
	blob_panel = label_panel(make_mask_bgr(state["fg_mask"]), "Blob Filtered")
	overlay_panel = depth_bgr.copy()
	if state["fg_mask"] is not None:
		overlay_panel[state["fg_mask"] > 0] = (0, 0, 255)
	overlay_panel = label_panel(overlay_panel, "Final Overlay")

	row1 = np.hstack((depth_panel, diff_panel, candidate_panel))
	row2 = np.hstack((blur_panel, blob_panel, overlay_panel))
	return np.vstack((row1, row2))


def on_mouse(event, x, y, flags, state):
	del flags
	if event != cv2.EVENT_LBUTTONDOWN:
		return

	depth_m = state["depth_m"]
	if depth_m is None:
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

	base = "PAUSED" if state["paused"] else "LIVE"
	mode = "ARMED" if state["ref_depth_m"] is not None else "UNARMED"
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
		f"fg_pixels={state['foreground_pixels']} largest_blob={state['largest_blob_pixels']} smooth_blob={state['smoothed_blob_pixels']:.1f}",
		(10, 75),
		font_scale=0.50,
		thickness=1,
	)

	draw_outlined_text(
		view,
		f"trigger={state['trigger_metric_pixels']:.1f}/{BLOB_TRIGGER_PIXELS} min_blob_area={MIN_BLOB_AREA_PIXELS}",
		(10, 95),
		font_scale=0.50,
		thickness=1,
	)

	draw_outlined_text(
		view,
		f"depth_filter={'on' if ENABLE_DEPTH_PRE_FILTER else 'off'}({DEPTH_FILTER_KERNEL}) mask_blur={'on' if ENABLE_MASK_BLUR else 'off'}({MASK_BLUR_KERNEL}) kalman={'on' if ENABLE_KALMAN else 'off'}",
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
		# Highlight detected foreground in red over the depth map.
		view[mask_small > 0] = (0, 0, 255)

	if state["clicked_uv"] is not None and state["depth_m"] is not None:
		u, v = state["clicked_uv"]
		d = state["depth_m"][v, u]
		if np.isnan(d):
			click_text = f"click ({u},{v}) invalid"
		else:
			click_text = f"click ({u},{v}) depth={d:.3f} m / {d * 3.28084:.3f} ft"
		draw_outlined_text(view, click_text, (10, 155), font_scale=0.52, thickness=1)

	return view


def save_snapshot(state):
	if state["depth_m"] is None:
		print("No frame available to save yet.")
		return

	os.makedirs("captures", exist_ok=True)
	stamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
	npz_path = os.path.join("captures", f"intruder_{stamp}.npz")
	png_path = os.path.join("captures", f"intruder_{stamp}.png")

	np.savez_compressed(
		npz_path,
		dcs0=state["dcs_frames"][0],
		dcs1=state["dcs_frames"][1],
		dcs2=state["dcs_frames"][2],
		dcs3=state["dcs_frames"][3],
		phase_rad=state["phase_rad"],
		depth_m=state["depth_m"],
		ref_depth_m=state["ref_depth_m"],
		fg_mask=state["fg_mask"],
		smoothed_blob_pixels=state["smoothed_blob_pixels"],
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
	if state["depth_m"] is None:
		return

	invalid_mask = ~np.isfinite(state["depth_m"])
	depth_bgr = depth_to_colormap(state["depth_m"], invalid_mask, state["max_depth_m"])
	if state["show_workflow"]:
		state["display_bgr"] = build_workflow_view(depth_bgr, state)
	else:
		state["display_bgr"] = cv2.resize(
			depth_bgr,
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
		"phase_rad": None,
		"depth_m": None,
		"depth_filtered_m": None,
		"display_bgr": None,
		"modulation_hz": MODULATION_HZ,
		"max_depth_m": MAX_DEPTH_M,
		"depth_scale_m_per_rad": DEPTH_SCALE_M_PER_RAD,
		"ref_depth_m": None,
		"fg_mask": None,
		"candidate_mask": None,
		"post_blur_mask": None,
		"diff_m": None,
		"foreground_pixels": 0,
		"largest_blob_pixels": 0,
		"smoothed_blob_pixels": 0.0,
		"trigger_metric_pixels": 0.0,
		"blob_area_kf": None,
		"blob_missed_frames": 0,
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
								state["max_depth_m"], state["depth_scale_m_per_rad"] = compute_depth_params(new_mod_hz)
								log_event(
									f"Updated modulation from status: md={md} fLED={new_mod_hz / 1e6:.3f}MHz "
									f"Dmax={state['max_depth_m']:.3f}m"
								)
						log_event(status_line)
					next_status_poll = now + STATUS_POLL_INTERVAL_S

				if not state["paused"]:
					dcs_frames = request_one_dcs_quad(ser)
					if dcs_frames is None:
						log_event("Frame request timed out or incomplete")
						continue

					phase_rad, depth_m, invalid_mask = compute_phase_and_depth(
						dcs_frames,
						state["depth_scale_m_per_rad"],
						state["max_depth_m"],
					)

					update_intruder_detection(state, depth_m, invalid_mask)

					depth_bgr = depth_to_colormap(depth_m, invalid_mask, state["max_depth_m"])
					if state["show_workflow"]:
						display_bgr = build_workflow_view(depth_bgr, state)
					else:
						display_bgr = cv2.resize(
							depth_bgr,
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
