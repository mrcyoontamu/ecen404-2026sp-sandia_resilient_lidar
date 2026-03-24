import argparse
import time
from typing import Optional

import serial
from serial.tools import list_ports


HEADER = b"\xAA\x55\xAA\x55"


def find_port(preferred: Optional[str]) -> Optional[str]:
	if preferred:
		return preferred

	ports = list_ports.comports()
	for p in ports:
		desc = (p.description or "").lower()
		if "cdc" in desc or "usb" in desc or "virtual" in desc:
			return p.device
	return ports[0].device if ports else None


def read_payload(buffer: bytearray, ser: serial.Serial, length: int, timeout_s: float) -> Optional[bytes]:
	deadline = time.monotonic() + timeout_s
	while len(buffer) < length:
		if time.monotonic() > deadline:
			return None
		chunk = ser.read(length - len(buffer))
		if chunk:
			buffer.extend(chunk)
	return bytes(buffer[:length])


def verify_payload(seq: int, payload: bytes) -> bool:
	for i, b in enumerate(payload):
		if b != ((seq + i) & 0xFF):
			return False
	return True


def main() -> None:
	parser = argparse.ArgumentParser(description="STM32 USB CDC frame receiver")
	parser.add_argument("--port", default=None, help="Serial port (auto-detect if omitted)")
	parser.add_argument("--payload", type=int, default=4096,
						choices=[4096, 32768, 61440, 32764, 614400],
						help="Expected payload size")
	parser.add_argument("--trigger", action="store_true",
						help="Send 'G' to trigger one frame (for HOST_TRIGGER_MODE)")
	parser.add_argument("--trigger-period-ms", type=int, default=20,
						help="Period for sending 'G' when --trigger is set")
	args = parser.parse_args()

	frames_ok = 0
	frames_corrupt = 0
	sequence_gaps = 0
	inter_sum = 0.0
	inter_count = 0
	inter_max = 0.0

	last_seq = None
	last_frame_time = None

	last_print = time.monotonic()
	next_trigger = time.monotonic()

	buffer = bytearray()

	while True:
		port = find_port(args.port)
		if not port:
			print("No serial ports found. Retrying...")
			time.sleep(1.0)
			continue

		try:
			with serial.Serial(port, baudrate=115200, timeout=0.2) as ser:
				print(f"Connected to {port}")
				buffer.clear()

				while True:
					if args.trigger and time.monotonic() >= next_trigger:
						ser.write(b"G")
						next_trigger = time.monotonic() + (args.trigger_period_ms / 1000.0)

					chunk = ser.read(4096)
					if chunk:
						buffer.extend(chunk)

					# Find header
					while True:
						idx = buffer.find(HEADER)
						if idx < 0:
							if len(buffer) > 3:
								buffer[:] = buffer[-3:]
							break

						if idx > 0:
							del buffer[:idx]

						if len(buffer) < 12:
							break

						seq = int.from_bytes(buffer[4:8], "little")
						length = int.from_bytes(buffer[8:12], "little")
						del buffer[:12]

						payload = read_payload(buffer, ser, length, timeout_s=1.0)
						if payload is None:
							frames_corrupt += 1
							print("Short read while waiting for payload")
							break
						del buffer[:length]

						if length != args.payload:
							frames_corrupt += 1
							print(f"Length mismatch: expected {args.payload}, got {length}")
							continue

						if not verify_payload(seq, payload):
							frames_corrupt += 1
							print(f"Payload corrupt at seq {seq}")
							continue

						now = time.monotonic()
						if last_seq is not None:
							expected = (last_seq + 1) & 0xFFFFFFFF
							if seq != expected:
								gap = (seq - expected) & 0xFFFFFFFF
								sequence_gaps += gap if gap > 0 else 1
								print(f"Sequence gap: last {last_seq} now {seq}")

						if last_frame_time is not None:
							inter = now - last_frame_time
							inter_sum += inter
							inter_count += 1
							if inter > inter_max:
								inter_max = inter

						last_seq = seq
						last_frame_time = now
						frames_ok += 1

					if time.monotonic() - last_print >= 1.0:
						mean_inter = (inter_sum / inter_count) if inter_count else 0.0
						print(
							f"frames_ok={frames_ok} frames_corrupt={frames_corrupt} "
							f"sequence_gaps={sequence_gaps} mean_inter_s={mean_inter:.4f} "
							f"max_inter_s={inter_max:.4f}"
						)
						last_print = time.monotonic()

		except serial.SerialException as exc:
			print(f"Serial error: {exc}. Reconnecting...")
			time.sleep(1.0)


if __name__ == "__main__":
	main()
