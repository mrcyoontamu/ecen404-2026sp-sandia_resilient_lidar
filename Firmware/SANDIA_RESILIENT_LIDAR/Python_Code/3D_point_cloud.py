import os
import time
from datetime import datetime

import cv2
import numpy as np
import serial
from serial.tools import list_ports

try:
	import open3d as o3d
except ImportError:
	o3d = None


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

# Datasheet 9.2.2 guidance: amplitudes below ~25 LSB are not useful for distance.
AMPLITUDE_MIN_LSB = 25.0

# Distance offset (DOFFSET in datasheet equation [2]). Keep 0 until calibrated.
DISTANCE_OFFSET_M = -1.8

# For calibration, keep negative-offset results clamped at 0 m rather than wrapped
# to the unambiguous max range (which can look like ~20 ft at 24 MHz).
WRAP_WITH_UNAMBIGUOUS_RANGE = True

# Unambiguous range for iToF phase at modulation frequency f is c/(2f).
MAX_DEPTH_M = SPEED_OF_LIGHT_M_PER_S / (2.0 * MODULATION_HZ)
DEPTH_SCALE_M_PER_RAD = SPEED_OF_LIGHT_M_PER_S / (4.0 * np.pi * MODULATION_HZ)

# Known ST USB IDs and debug-port exclusion.
TARGET_VID = 0x0483
TARGET_PIDS = {0x5740}
EXCLUDED_PIDS = {0x374B, 0x374D, 0x374E, 0x3752, 0x3753}
EXCLUDED_KEYWORDS = ("stlink", "st-link", "debugger")

# Display options.
WINDOW_NAME = "Live ToF Point Cloud"
WINDOW_WIDTH = 1440
WINDOW_HEIGHT = 900
POINT_SIZE = 2.0

# Lens FoV input (replace with your real optics values).
LENS_FOV_HORIZONTAL_DEG = 106
LENS_FOV_VERTICAL_DEG = 79.5

# Rendering and filtering.
POINT_STRIDE = 2
MIN_DEPTH_M = 0.05
MAX_DEPTH_MARGIN_M = 0.0

# World orientation controls.
FLIP_X_AXIS = False
FLIP_Y_AXIS = True

CMD_GET_FRAME = b"G"
CMD_STATUS = b"S"


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

	# Match MCU path: EPC660 data is 12-bit with 0x0FFF saturation code.
	raw0 = f0 & 0x0FFF
	raw1 = f1 & 0x0FFF
	raw2 = f2 & 0x0FFF
	raw3 = f3 & 0x0FFF

	sat_mask = (raw0 == 0x0FFF) | (raw1 == 0x0FFF) | (raw2 == 0x0FFF) | (raw3 == 0x0FFF)

	# Match firmware calculate_depth_simple(): center around EPC660 midpoint (2048)
	# before building I/Q vectors.
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

	return phase_rad, depth_m, sat_mask, low_signal_mask


def depth_to_colormap(depth_m, invalid_mask, max_depth_m):
	clipped = np.clip(depth_m, 0.0, max_depth_m)
	norm = np.nan_to_num(clipped / max_depth_m, nan=0.0)
	gray = (norm * 255.0).astype(np.uint8)
	bgr = cv2.applyColorMap(gray, cv2.COLORMAP_TURBO)
	bgr[invalid_mask] = (0, 0, 0)
	return bgr


def build_xy_scale_lut(width, height, fov_h_deg, fov_v_deg):
	fov_h_rad = np.deg2rad(fov_h_deg)
	fov_v_rad = np.deg2rad(fov_v_deg)

	if fov_h_rad <= 0.0 or fov_h_rad >= np.pi or fov_v_rad <= 0.0 or fov_v_rad >= np.pi:
		raise ValueError("FoV must be in (0, 180) degrees.")

	fx = (width * 0.5) / np.tan(fov_h_rad * 0.5)
	fy = (height * 0.5) / np.tan(fov_v_rad * 0.5)
	cx = (width - 1) * 0.5
	cy = (height - 1) * 0.5

	u = np.arange(width, dtype=np.float32)
	v = np.arange(height, dtype=np.float32)
	u_grid, v_grid = np.meshgrid(u, v)

	x_scale = (u_grid - cx) / fx
	y_scale = (v_grid - cy) / fy

	if FLIP_X_AXIS:
		x_scale = -x_scale
	if FLIP_Y_AXIS:
		y_scale = -y_scale

	return x_scale.astype(np.float32), y_scale.astype(np.float32)


def depth_to_point_cloud(depth_m, invalid_mask, max_depth_m, x_scale, y_scale):
	depth_cap = max_depth_m + MAX_DEPTH_MARGIN_M
	valid = (~invalid_mask) & np.isfinite(depth_m)
	valid &= depth_m >= MIN_DEPTH_M
	valid &= depth_m <= depth_cap

	if POINT_STRIDE > 1:
		stride_mask = np.zeros_like(valid)
		stride_mask[::POINT_STRIDE, ::POINT_STRIDE] = True
		valid &= stride_mask

	if not np.any(valid):
		return np.empty((0, 3), dtype=np.float32), np.empty((0, 3), dtype=np.float32)

	x = depth_m * x_scale
	y = depth_m * y_scale
	# Open3D default view looks toward -Z; place measured depth in front of camera.
	z = -depth_m

	xyz = np.stack((x[valid], y[valid], z[valid]), axis=1).astype(np.float32)

	depth_rgb_bgr = depth_to_colormap(depth_m, ~valid, max_depth_m)
	depth_rgb = depth_rgb_bgr[:, :, ::-1].astype(np.float32) / 255.0
	colors = depth_rgb[valid]

	return xyz, colors


def log_controls_once():
	print("Open3D controls:")
	print("  Mouse drag: rotate")
	print("  Shift + mouse drag: pan")
	print("  Mouse wheel: zoom")
	print("  Space: pause/resume capture")
	print("  R: reset camera to fit cloud")
	print("  C: probe center pixel")
	print("  S: save snapshot (.npz + .ply + .png)")
	print("  Q: quit")


def probe_center_pixel(state):
	depth_m = state["depth_m"]
	if depth_m is None:
		print("No frame available yet.")
		return

	u = FRAME_WIDTH // 2
	v = FRAME_HEIGHT // 2
	d = depth_m[v, u]
	p = state["phase_rad"][v, u]
	f0 = int(state["dcs_frames"][0][v, u])
	f1 = int(state["dcs_frames"][1][v, u])
	f2 = int(state["dcs_frames"][2][v, u])
	f3 = int(state["dcs_frames"][3][v, u])

	if np.isnan(d):
		print(f"Center ({u}, {v}) -> invalid/saturated pixel")
	else:
		print(
			f"Center ({u}, {v}) -> depth={d:.4f} m ({d * 3.28084:.3f} ft), "
			f"phase={p:.4f} rad, DCS=[{f0}, {f1}, {f2}, {f3}]"
		)


def save_snapshot(state):
	if state["depth_m"] is None:
		print("No frame available to save yet.")
		return

	os.makedirs("captures", exist_ok=True)
	stamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
	npz_path = os.path.join("captures", f"capture_{stamp}.npz")
	ply_path = os.path.join("captures", f"capture_{stamp}.ply")
	png_path = os.path.join("captures", f"capture_{stamp}.png")

	np.savez_compressed(
		npz_path,
		dcs0=state["dcs_frames"][0],
		dcs1=state["dcs_frames"][1],
		dcs2=state["dcs_frames"][2],
		dcs3=state["dcs_frames"][3],
		phase_rad=state["phase_rad"],
		depth_m=state["depth_m"],
		modulation_hz=state["modulation_hz"],
		xyz=state["pc_xyz"],
		rgb=state["pc_rgb"],
	)

	pcd = o3d.geometry.PointCloud()
	pcd.points = o3d.utility.Vector3dVector(state["pc_xyz"].astype(np.float64))
	pcd.colors = o3d.utility.Vector3dVector(state["pc_rgb"].astype(np.float64))
	o3d.io.write_point_cloud(ply_path, pcd, write_ascii=False, compressed=True)

	if state["vis"] is not None:
		state["vis"].capture_screen_image(png_path, do_render=True)

	print(f"Saved: {npz_path}")
	print(f"Saved: {ply_path}")
	print(f"Saved: {png_path}")


def build_visualizer(state):
	vis = o3d.visualization.VisualizerWithKeyCallback()
	vis.create_window(WINDOW_NAME, WINDOW_WIDTH, WINDOW_HEIGHT)

	pcloud = o3d.geometry.PointCloud()
	vis.add_geometry(pcloud)

	render_opt = vis.get_render_option()
	render_opt.background_color = np.array([0.03, 0.03, 0.04], dtype=np.float64)
	render_opt.point_size = float(POINT_SIZE)
	render_opt.show_coordinate_frame = True

	def on_quit(_vis):
		state["quit"] = True
		return False

	def on_pause(_vis):
		state["paused"] = not state["paused"]
		if state["ser"] is not None:
			flush_serial_input(state["ser"])
			state["next_status_poll"] = time.monotonic()
		log_event("PAUSE" if state["paused"] else "RESUME")
		return False

	def on_save(_vis):
		save_snapshot(state)
		return False

	def on_probe(_vis):
		probe_center_pixel(state)
		return False

	def on_reset_view(_vis):
		if state["pc_xyz"].shape[0] > 0:
			_vis.reset_view_point(True)
			log_event("Camera reset to fit current cloud")
		return False

	vis.register_key_callback(ord("Q"), on_quit)
	vis.register_key_callback(ord("q"), on_quit)
	vis.register_key_callback(ord(" "), on_pause)
	vis.register_key_callback(ord("S"), on_save)
	vis.register_key_callback(ord("s"), on_save)
	vis.register_key_callback(ord("C"), on_probe)
	vis.register_key_callback(ord("c"), on_probe)
	vis.register_key_callback(ord("R"), on_reset_view)
	vis.register_key_callback(ord("r"), on_reset_view)

	return vis, pcloud


def main():
	if o3d is None:
		print("open3d is required for the 3D point cloud viewer.")
		print("Install it with: pip install open3d")
		return

	x_scale, y_scale = build_xy_scale_lut(
		FRAME_WIDTH,
		FRAME_HEIGHT,
		LENS_FOV_HORIZONTAL_DEG,
		LENS_FOV_VERTICAL_DEG,
	)

	state = {
		"paused": False,
		"quit": False,
		"dcs_frames": None,
		"phase_rad": None,
		"depth_m": None,
		"pc_xyz": np.empty((0, 3), dtype=np.float32),
		"pc_rgb": np.empty((0, 3), dtype=np.float32),
		"vis": None,
		"pcd": None,
		"ser": None,
		"next_status_poll": 0.0,
		"modulation_hz": MODULATION_HZ,
		"max_depth_m": MAX_DEPTH_M,
		"depth_scale_m_per_rad": DEPTH_SCALE_M_PER_RAD,
		"fps": 0.0,
		"fps_frame_counter": 0,
		"fps_last_t": time.monotonic(),
		"fps_last_print_t": time.monotonic(),
		"view_initialized": False,
	}

	state["vis"], state["pcd"] = build_visualizer(state)
	log_controls_once()
	log_event(
		f"FoV: H={LENS_FOV_HORIZONTAL_DEG:.2f} deg, V={LENS_FOV_VERTICAL_DEG:.2f} deg | "
		f"stride={POINT_STRIDE}"
	)

	ser = None
	next_status_poll = 0.0

	try:
		while True:
			if state["quit"]:
				break

			if ser is None:
				ser = connect_with_retry(BAUD, TIMEOUT)
				next_status_poll = time.monotonic()
				state["ser"] = ser
				state["next_status_poll"] = next_status_poll
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
					state["next_status_poll"] = next_status_poll

				if not state["paused"]:
					dcs_frames = request_one_dcs_quad(ser)
					if dcs_frames is None:
						log_event("Frame request timed out or incomplete")
						if not state["vis"].poll_events():
							state["quit"] = True
						state["vis"].update_renderer()
						continue

					phase_rad, depth_m, sat_mask, low_signal_mask = compute_phase_and_depth(
						dcs_frames,
						state["depth_scale_m_per_rad"],
						state["max_depth_m"],
					)

					xyz, rgb = depth_to_point_cloud(
						depth_m,
						sat_mask | low_signal_mask,
						state["max_depth_m"],
						x_scale,
						y_scale,
					)

					state["dcs_frames"] = dcs_frames
					state["phase_rad"] = phase_rad
					state["depth_m"] = depth_m
					state["pc_xyz"] = xyz
					state["pc_rgb"] = rgb

					state["pcd"].points = o3d.utility.Vector3dVector(xyz.astype(np.float64))
					state["pcd"].colors = o3d.utility.Vector3dVector(rgb.astype(np.float64))
					state["vis"].update_geometry(state["pcd"])

					if (not state["view_initialized"]) and xyz.shape[0] > 0:
						state["vis"].reset_view_point(True)
						state["view_initialized"] = True

					state["fps_frame_counter"] += 1
					now_fps = time.monotonic()
					dt = now_fps - state["fps_last_t"]
					if dt >= 0.5:
						state["fps"] = state["fps_frame_counter"] / dt
						state["fps_frame_counter"] = 0
						state["fps_last_t"] = now_fps

					dt_print = now_fps - state["fps_last_print_t"]
					if dt_print >= 1.0:
						print(
							f"FPS {state['fps']:.1f} | points {xyz.shape[0]} | "
							f"mod {state['modulation_hz'] / 1e6:.2f}MHz"
						)
						state["fps_last_print_t"] = now_fps

				if not state["vis"].poll_events():
					state["quit"] = True
					break
				state["vis"].update_renderer()

				if state["paused"]:
					time.sleep(0.02)

			except (serial.SerialException, OSError) as exc:
				log_event(f"Serial link lost: {exc}")
				try:
					ser.close()
				except Exception:
					pass
				ser = None
				state["ser"] = None
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
		if state.get("vis") is not None:
			state["vis"].destroy_window()


if __name__ == "__main__":
	main()
