from pathlib import Path
import argparse

import cv2
import numpy as np


MIDPOINT_LSB = 2048.0
DEFAULT_WINDOW_NAME = "DCS Raw Amplitude (2x2)"

# BGR colors so each DCS tile is visually distinct.
DCS_TINTS_BGR = {
    "dcs0": np.array([0, 0, 255], dtype=np.float32),      # red
    "dcs1": np.array([255, 0, 0], dtype=np.float32),      # blue
    "dcs2": np.array([0, 200, 0], dtype=np.float32),      # green
    "dcs3": np.array([0, 180, 255], dtype=np.float32),    # orange
}


def _default_captures_dir() -> Path:
    return Path(__file__).resolve().parent / "captures"


def _resolve_capture_path(capture_id: str | None = None, captures_dir: str | Path | None = None) -> Path:
    base_dir = Path(captures_dir) if captures_dir is not None else _default_captures_dir()
    base_dir = base_dir.resolve()

    if not base_dir.exists():
        raise FileNotFoundError(f"Captures directory does not exist: {base_dir}")

    if capture_id is None or str(capture_id).strip() == "":
        candidates = sorted(base_dir.glob("capture_*.npz"), key=lambda p: p.stat().st_mtime)
        if not candidates:
            raise FileNotFoundError(f"No capture_*.npz files found in {base_dir}")
        return candidates[-1]

    capture_id = str(capture_id).strip()

    # Accept an explicit file path.
    explicit = Path(capture_id)
    if explicit.suffix.lower() == ".npz" and explicit.exists():
        return explicit.resolve()

    # Accept both raw IDs and prefixed names.
    if capture_id.endswith(".npz"):
        filename = capture_id
    elif capture_id.startswith("capture_"):
        filename = f"{capture_id}.npz"
    else:
        filename = f"capture_{capture_id}.npz"

    candidate = base_dir / filename
    if not candidate.exists():
        raise FileNotFoundError(f"Capture file not found: {candidate}")
    return candidate.resolve()


def _to_amplitude_map_u8(frame_u16: np.ndarray) -> np.ndarray:
    raw12 = frame_u16.astype(np.float32) & 0x0FFF
    amp = np.abs(raw12 - MIDPOINT_LSB)

    peak = float(np.max(amp))
    if peak <= 0.0:
        return np.zeros_like(frame_u16, dtype=np.uint8)

    norm = np.clip(amp / peak, 0.0, 1.0)
    return (norm * 255.0).astype(np.uint8)


def _tint_gray(gray_u8: np.ndarray, tint_bgr: np.ndarray) -> np.ndarray:
    scale = (gray_u8.astype(np.float32) / 255.0)[..., None]
    colored = scale * tint_bgr[None, None, :]
    return np.clip(colored, 0, 255).astype(np.uint8)


def _label_tile(tile_bgr: np.ndarray, label: str) -> np.ndarray:
    out = tile_bgr.copy()
    cv2.putText(out, label, (10, 24), cv2.FONT_HERSHEY_SIMPLEX, 0.65, (255, 255, 255), 2, cv2.LINE_AA)
    return out


def _make_grid(tile00: np.ndarray, tile01: np.ndarray, tile10: np.ndarray, tile11: np.ndarray) -> np.ndarray:
    top = np.hstack([tile00, tile01])
    bottom = np.hstack([tile10, tile11])
    return np.vstack([top, bottom])


def display_dcs_frames_raw(
    capture_id: str | None = None,
    captures_dir: str | Path | None = None,
    scale: int = 2,
    window_name: str = DEFAULT_WINDOW_NAME,
) -> Path:
    """Display DCS0..DCS3 amplitude maps in a color-tinted 2x2 grid.

    capture_id:
      - None or "" => load latest capture_*.npz
      - "YYYYMMDD_HHMMSS_xxxxxx" => load capture_<id>.npz
      - "capture_..." => load capture_....npz
      - explicit .npz path => load that file
    """
    if scale < 1:
        raise ValueError("scale must be >= 1")

    capture_path = _resolve_capture_path(capture_id=capture_id, captures_dir=captures_dir)

    with np.load(capture_path) as data:
        required = ("dcs0", "dcs1", "dcs2", "dcs3")
        missing = [name for name in required if name not in data]
        if missing:
            raise KeyError(f"Capture missing expected arrays: {missing}")

        dcs0 = data["dcs0"]
        dcs1 = data["dcs1"]
        dcs2 = data["dcs2"]
        dcs3 = data["dcs3"]

    tiles = []
    for key, frame in (("dcs0", dcs0), ("dcs1", dcs1), ("dcs2", dcs2), ("dcs3", dcs3)):
        gray = _to_amplitude_map_u8(frame)
        color = _tint_gray(gray, DCS_TINTS_BGR[key])
        labeled = _label_tile(color, key.upper())
        if scale != 1:
            h, w = labeled.shape[:2]
            labeled = cv2.resize(labeled, (w * scale, h * scale), interpolation=cv2.INTER_NEAREST)
        tiles.append(labeled)

    grid = _make_grid(tiles[0], tiles[1], tiles[2], tiles[3])

    print(f"Displaying: {capture_path}")
    cv2.namedWindow(window_name, cv2.WINDOW_NORMAL)
    cv2.imshow(window_name, grid)
    print("Press any key in the image window to close.")
    cv2.waitKey(0)
    cv2.destroyWindow(window_name)

    return capture_path


def _build_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Display raw DCS amplitude maps in a colorized 2x2 grid")
    parser.add_argument(
        "capture_id",
        nargs="?",
        default=None,
        help="Capture ID (timestamp suffix), capture_*.npz name, or explicit .npz path. Omit for latest.",
    )
    parser.add_argument(
        "--captures-dir",
        default=None,
        help="Optional captures directory. Defaults to Python_Code/captures next to this script.",
    )
    parser.add_argument("--scale", type=int, default=2, help="Integer display scaling factor (default: 2)")
    return parser


def main() -> None:
    args = _build_arg_parser().parse_args()
    display_dcs_frames_raw(capture_id=args.capture_id, captures_dir=args.captures_dir, scale=args.scale)


if __name__ == "__main__":
    main()
