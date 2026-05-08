import argparse
import csv
import json
import math
import os
from typing import Dict, List, Optional


def parse_float(value: str) -> Optional[float]:
    if value is None:
        return None
    value = str(value).strip()
    if value == "":
        return None
    try:
        num = float(value)
    except ValueError:
        return None
    if not math.isfinite(num):
        return None
    return num


def percentile(values: List[float], q: float) -> Optional[float]:
    if not values:
        return None
    sorted_vals = sorted(values)
    idx = int(math.floor((len(sorted_vals) - 1) * q))
    idx = max(0, min(idx, len(sorted_vals) - 1))
    return sorted_vals[idx]


def mean(values: List[float]) -> Optional[float]:
    if not values:
        return None
    return sum(values) / float(len(values))


def summarize_series(values: List[float]) -> Dict[str, Optional[float]]:
    return {
        "mean": mean(values),
        "q95": percentile(values, 0.95),
        "q99": percentile(values, 0.99),
        "q999": percentile(values, 0.999),
        "max": max(values) if values else None,
    }


def fmt_or_na(value: Optional[float]) -> str:
    if value is None:
        return "n/a"
    return f"{value:.6f}"


def to_int(value: Optional[float], default: int = 0) -> int:
    if value is None:
        return default
    return int(round(value))


def load_frames_csv(path: str) -> List[Dict[str, str]]:
    with open(path, "r", newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def load_manifest(path: str) -> Dict:
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def collect_numeric(rows: List[Dict[str, str]], key: str) -> List[float]:
    out: List[float] = []
    for row in rows:
        val = parse_float(row.get(key, ""))
        if val is not None:
            out.append(val)
    return out


def summarize_run(run_dir: str) -> Dict:
    frames_path = os.path.join(run_dir, "frames.csv")
    manifest_path = os.path.join(run_dir, "manifest.json")

    rows = load_frames_csv(frames_path)
    manifest = load_manifest(manifest_path)
    cfg = manifest.get("runtime_config", {})

    unix_times = collect_numeric(rows, "unix_time_s")
    triggered = collect_numeric(rows, "triggered")
    foreground = collect_numeric(rows, "foreground_pixels")
    largest_blob = collect_numeric(rows, "largest_blob_pixels")
    trigger_metric = collect_numeric(rows, "trigger_metric_pixels")
    depth_candidate = collect_numeric(rows, "depth_candidate_pixels")
    low_conf_reject = collect_numeric(rows, "low_confidence_reject_pixels")
    sat_transition = collect_numeric(rows, "sat_transition_pixels")
    diff_p95 = collect_numeric(rows, "diff_p95_m")
    diff_p99 = collect_numeric(rows, "diff_p99_m")
    diff_p999 = collect_numeric(rows, "diff_p999_m")
    diff_mean = collect_numeric(rows, "diff_mean_m")
    invalid_ratio = collect_numeric(rows, "invalid_ratio")

    duration_s = 0.0
    fps = 0.0
    if len(unix_times) >= 2:
        duration_s = max(0.0, unix_times[-1] - unix_times[0])
        if duration_s > 0.0:
            fps = len(rows) / duration_s

    sum_depth_candidate = sum(depth_candidate) if depth_candidate else 0.0
    sum_low_conf_reject = sum(low_conf_reject) if low_conf_reject else 0.0
    low_conf_reject_ratio = 0.0
    if sum_depth_candidate > 0.0:
        low_conf_reject_ratio = sum_low_conf_reject / sum_depth_candidate

    diff_q999 = percentile(diff_p95, 0.999) or 0.0
    rec_diff = max(float(cfg.get("DIFF_THRESHOLD_M", 0.1)), round(diff_q999 * 1.20, 4))

    blob_q999 = percentile(largest_blob, 0.999) or 0.0
    rec_blob_trigger = max(int(cfg.get("BLOB_TRIGGER_PIXELS", 30)), int(math.ceil(blob_q999 + 5.0)))

    blob_area_q999 = percentile(foreground, 0.999) or 0.0
    rec_min_blob_area = max(int(cfg.get("MIN_BLOB_AREA_PIXELS", 25)), int(math.ceil(blob_area_q999 + 3.0)))

    summary = {
        "run_dir": run_dir,
        "frame_count": len(rows),
        "duration_s": duration_s,
        "approx_fps": fps,
        "triggered_frames": to_int(sum(triggered), 0),
        "max_foreground_pixels": to_int(max(foreground) if foreground else 0.0, 0),
        "max_largest_blob_pixels": to_int(max(largest_blob) if largest_blob else 0.0, 0),
        "max_trigger_metric_pixels": max(trigger_metric) if trigger_metric else 0.0,
        "max_depth_candidate_pixels": to_int(max(depth_candidate) if depth_candidate else 0.0, 0),
        "max_low_confidence_reject_pixels": to_int(max(low_conf_reject) if low_conf_reject else 0.0, 0),
        "max_sat_transition_pixels": to_int(max(sat_transition) if sat_transition else 0.0, 0),
        "diff_p95_stats_m": summarize_series(diff_p95),
        "diff_p99_stats_m": summarize_series(diff_p99),
        "diff_p999_stats_m": summarize_series(diff_p999),
        "diff_mean_run_m": mean(diff_mean),
        "diff_mean_run_sample_count": len(diff_mean),
        "invalid_ratio_mean": mean(invalid_ratio),
        "low_confidence_reject_ratio": low_conf_reject_ratio,
        "current_config": {
            "DIFF_THRESHOLD_M": cfg.get("DIFF_THRESHOLD_M"),
            "BLOB_TRIGGER_PIXELS": cfg.get("BLOB_TRIGGER_PIXELS"),
            "MIN_BLOB_AREA_PIXELS": cfg.get("MIN_BLOB_AREA_PIXELS"),
            "AUTO_BACKGROUND_UPDATE_FRAMES": cfg.get("AUTO_BACKGROUND_UPDATE_FRAMES"),
        },
        "recommendations_from_empty_room": {
            "DIFF_THRESHOLD_M": rec_diff,
            "BLOB_TRIGGER_PIXELS": rec_blob_trigger,
            "MIN_BLOB_AREA_PIXELS": rec_min_blob_area,
            "AUTO_BACKGROUND_UPDATE_FRAMES_for_baseline": 0,
            "notes": [
                "This run is empty-room baseline only; use intrusion runs before lowering sensitivity.",
                "Raise DIFF_THRESHOLD_M if nuisance alarms persist in baseline or nuisance-only runs.",
                "Keep BLOB_TRIGGER_PIXELS and MIN_BLOB_AREA_PIXELS conservative until intrusion data is analyzed.",
            ],
        },
    }

    return summary


def main() -> None:
    parser = argparse.ArgumentParser(description="Analyze one diagnostics run folder and output baseline tuning guidance.")
    parser.add_argument("run_dir", help="Path to a diagnostics run directory that contains frames.csv and manifest.json")
    args = parser.parse_args()

    run_dir = os.path.abspath(args.run_dir)
    summary = summarize_run(run_dir)

    out_path = os.path.join(run_dir, "analysis_summary.json")
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(summary, f, indent=2)

    print(f"Wrote analysis summary: {out_path}")
    print("--- Core Stats ---")
    print(f"frames={summary['frame_count']} duration_s={summary['duration_s']:.2f} fps={summary['approx_fps']:.3f}")
    print(
        "triggered_frames="
        f"{summary['triggered_frames']} max_blob={summary['max_largest_blob_pixels']} "
        f"max_fg={summary['max_foreground_pixels']}"
    )
    diff_stats = summary["diff_p95_stats_m"]
    diff_p99_stats = summary["diff_p99_stats_m"]
    diff_p999_stats = summary["diff_p999_stats_m"]
    print(
        "diff_mean_run_m: "
        f"value={fmt_or_na(summary['diff_mean_run_m'])} samples={summary['diff_mean_run_sample_count']}"
    )
    print(
        "diff_p95_m: "
        f"mean={fmt_or_na(diff_stats['mean'])} q95={fmt_or_na(diff_stats['q95'])} "
        f"q99={fmt_or_na(diff_stats['q99'])} q999={fmt_or_na(diff_stats['q999'])}"
    )
    print(
        "diff_p99_m: "
        f"mean={fmt_or_na(diff_p99_stats['mean'])} q95={fmt_or_na(diff_p99_stats['q95'])} "
        f"q99={fmt_or_na(diff_p99_stats['q99'])} q999={fmt_or_na(diff_p99_stats['q999'])}"
    )
    print(
        "diff_p999_m: "
        f"mean={fmt_or_na(diff_p999_stats['mean'])} q95={fmt_or_na(diff_p999_stats['q95'])} "
        f"q99={fmt_or_na(diff_p999_stats['q99'])} q999={fmt_or_na(diff_p999_stats['q999'])}"
    )
    rec = summary["recommendations_from_empty_room"]
    print("--- Recommended Starting Values (from baseline only) ---")
    print(f"DIFF_THRESHOLD_M={rec['DIFF_THRESHOLD_M']}")
    print(f"BLOB_TRIGGER_PIXELS={rec['BLOB_TRIGGER_PIXELS']}")
    print(f"MIN_BLOB_AREA_PIXELS={rec['MIN_BLOB_AREA_PIXELS']}")


if __name__ == "__main__":
    main()
