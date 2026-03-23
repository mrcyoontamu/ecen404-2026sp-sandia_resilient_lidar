import serial
from serial.tools import list_ports
import numpy as np
import cv2
import time

# --- CONFIGURATION ---
# Set FORCE_PORT to a COM string (e.g., 'COM7') to bypass auto-discovery.
FORCE_PORT = None
BAUD = 921600
FRAME_WIDTH = 320
FRAME_HEIGHT = 240
FRAME_SIZE = FRAME_WIDTH * FRAME_HEIGHT * 2
HEADER = b'\xAA\x55\xAA\x55'
TIMEOUT = 5
RECONNECT_INTERVAL_S = 5

# STM defaults for USB CDC ACM in many STM32 examples.
TARGET_VID = 0x0483
TARGET_PIDS = {0x5740}

# Known STLink VCP PIDs to avoid when auto-selecting target device.
EXCLUDED_PIDS = {0x374B, 0x374D, 0x374E, 0x3752, 0x3753}

# Description/HWID keywords to reject (debugger-side virtual COM).
EXCLUDED_KEYWORDS = (
    "stlink",
    "st-link",
    "debugger",
)

# Display mode options:
# - "phase_color": cyclic color map for 16-bit angle-like values (same value -> same color)
# - "linear_gray": fixed linear grayscale mapping over 0..65535
DISPLAY_MODE = "phase_color"

def list_available_ports():
    ports = sorted([p.device for p in list_ports.comports()])
    return ", ".join(ports) if ports else "none"


def pick_target_port():
    """Pick the best candidate COM port for the camera USB CDC device."""
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

    # 1) Strong match: exact VID/PID.
    for p in ports:
        if is_excluded(p):
            continue
        if p.vid == TARGET_VID and p.pid in TARGET_PIDS:
            return p.device

    # 2) Soft match: known STM/CDC naming patterns.
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
    """Try connecting forever until success or user cancels with Ctrl+C."""
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
    """Find the 4-byte frame header in the serial stream."""
    buffer = b''
    while True:
        byte = ser.read(1)
        if not byte:
            return None  # timed out, no data
        buffer += byte
        if len(buffer) > 4:
            buffer = buffer[-4:]
        if buffer == HEADER:
            return True


def read_frame(ser):
    """Read one frame after finding the header. Returns bytes or None."""
    if not find_header(ser):
        return None
    frame_data = ser.read(FRAME_SIZE)
    if len(frame_data) != FRAME_SIZE:
        print(f"⚠️ Incomplete frame ({len(frame_data)} bytes)")
        return None
    return frame_data


def render_phase_color(frame_u16, sat_mask):
    """Render 16-bit values as smooth cyclic phase colors with a fixed mapping."""
    phase = frame_u16.astype(np.float32) * (2.0 * np.pi / 65536.0)

    # Smooth cyclic palette using three phase-shifted cosines.
    r = 0.5 + 0.5 * np.cos(phase)
    g = 0.5 + 0.5 * np.cos(phase - (2.0 * np.pi / 3.0))
    b = 0.5 + 0.5 * np.cos(phase - (4.0 * np.pi / 3.0))

    bgr = np.stack((b, g, r), axis=-1)
    bgr = np.clip(bgr * 255.0, 0, 255).astype(np.uint8)

    # Mark saturated pixels in black for easier debugging.
    bgr[sat_mask] = (0, 0, 0)
    return bgr


def render_linear_gray(frame_u16, sat_mask):
    """Render 16-bit values with fixed linear grayscale mapping over 0..65535."""
    disp = (frame_u16.astype(np.uint32) * 255 // 65535).astype(np.uint8)
    disp[sat_mask] = 0
    return disp

# --- MAIN LOOP ---
ser = None

try:
    while True:
        if ser is None:
            ser = connect_with_retry(BAUD, TIMEOUT)

        try:
            frame_bytes = read_frame(ser)
            if frame_bytes is None:
                continue  # timeout or partial read; keep waiting/resyncing

            # 1. Convert from raw bytes to a 2D numpy array
            raw_frame = np.frombuffer(frame_bytes, dtype='<u2').reshape((FRAME_HEIGHT, FRAME_WIDTH))

            # -----------------------------------------------------------------
            # --- EPC660 ROW UN-INTERLEAVING ---
            # -----------------------------------------------------------------
            # Even rows (0, 2, 4...) are the top half, moving outwards (up)
            top_half = raw_frame[0::2][::-1, :]

            # Odd rows (1, 3, 5...) are the bottom half, moving outwards (down)
            bottom_half = raw_frame[1::2, :]

            # Stack the top half and bottom half back together sequentially
            frame = np.vstack((top_half, bottom_half))

            # Flip vertically to restore desired image orientation.
            frame = np.flipud(frame)
            # -----------------------------------------------------------------

            sat_mask = (frame == 0xFFFF)

            if DISPLAY_MODE == "phase_color":
                disp = render_phase_color(frame, sat_mask)
            else:
                disp = render_linear_gray(frame, sat_mask)

            cv2.imshow("Camera", disp)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

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