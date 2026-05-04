#!/usr/bin/env python3
"""Compare RG Nano root-note detection with stronger desktop estimators.

The RG Nano app intentionally uses a small on-device pitch heuristic. This audit
keeps that heuristic honest by mirroring it in Python and comparing it against
desktop pYIN/YIN estimates from librosa on the same sample window.
"""

from __future__ import annotations

import argparse
import csv
import math
import tempfile
import wave
from dataclasses import dataclass
from pathlib import Path

import librosa
import numpy as np
import soundfile as sf

try:
    from swift_f0 import SwiftF0
except Exception:  # pragma: no cover - optional desktop QA dependency
    SwiftF0 = None  # type: ignore[assignment]


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SAMPLE_DIR = ROOT / "projects" / "rgnano-sim-data" / "samples"

NOTE_NAMES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
PERCUSSIVE_TOKENS = {
    "kick",
    "snare",
    "hat",
    "hihat",
    "clap",
    "rim",
    "tom",
    "cym",
    "crash",
    "ride",
    "drum",
    "perc",
    "shaker",
    "noise",
    "vinyl",
    "fx",
}


@dataclass
class Detection:
    midi: int | None
    freq: float | None
    confidence: float | None
    method: str
    skipped: str = ""


@dataclass
class Case:
    label: str
    path: Path
    expected_midi: int | None = None
    start: int = 0
    end: int | None = None


def note_name(midi: int | None) -> str:
    if midi is None:
        return "--"
    return f"{NOTE_NAMES[midi % 12]}{midi // 12 - 1}"


def freq_to_midi(freq: float | None) -> int | None:
    if not freq or freq <= 0.0 or not math.isfinite(freq):
        return None
    return max(0, min(127, int(69.0 + 12.0 * math.log2(freq / 440.0) + 0.5)))


def midi_to_freq(midi: int) -> float:
    return 440.0 * (2.0 ** ((midi - 69) / 12.0))


def cents_between(a: int | None, b: int | None) -> int | None:
    if a is None or b is None:
        return None
    return int((a - b) * 100)


def is_boundary(ch: str) -> bool:
    return not ch or not ch.isalnum()


def contains_token(name: str, token: str) -> bool:
    lower = name.lower()
    token = token.lower()
    start = 0
    while True:
        idx = lower.find(token, start)
        if idx < 0:
            return False
        before = lower[idx - 1] if idx > 0 else ""
        after_idx = idx + len(token)
        after = lower[after_idx] if after_idx < len(lower) else ""
        if is_boundary(before) and is_boundary(after):
            return True
        start = idx + 1


def looks_percussive(path: Path) -> bool:
    return any(contains_token(path.name, token) for token in PERCUSSIVE_TOKENS)


def read_mono(path: Path) -> tuple[np.ndarray, int]:
    data, sr = sf.read(str(path), always_2d=True)
    if data.size == 0:
        return np.zeros(0, dtype=np.float64), sr
    mono = data[:, 0].astype(np.float64)
    if mono.dtype.kind == "f":
        mono = np.clip(mono, -1.0, 1.0) * 32767.0
    return mono, int(sr)


def rg_correlation(buffer: np.ndarray, start: int, window: int, lag: int) -> float:
    a = buffer[start : start + window - lag]
    b = buffer[start + lag : start + window]
    e1 = float(np.dot(a, a))
    e2 = float(np.dot(b, b))
    if e1 <= 0.0 or e2 <= 0.0:
        return 0.0
    return float(np.dot(a, b) / math.sqrt(e1 * e2))


def detect_rg_nano(path: Path, start_frame: int = 0, end_frame: int | None = None) -> Detection:
    if looks_percussive(path):
        return Detection(None, None, None, "rg-nano", "percussive filename")

    buffer, sr = read_mono(path)
    size = len(buffer)
    if size < 256 or sr <= 0:
        return Detection(None, None, None, "rg-nano", "too short")

    range_start = max(0, min(start_frame, size - 1))
    range_end = size if end_frame is None or end_frame <= range_start else min(end_frame, size)
    range_size = range_end - range_start
    if range_size < 256:
        return Detection(None, None, None, "rg-nano", "trim too short")

    scan = min(range_size, sr)
    scan_window = buffer[range_start : range_start + scan]
    peak = int(np.max(np.abs(scan_window))) if len(scan_window) else 0
    if peak < 512:
        return Detection(None, None, None, "rg-nano", "low peak")
    rms = float(np.sqrt(np.mean(scan_window * scan_window)))
    if rms < 256.0:
        return Detection(None, None, None, "rg-nano", "low rms")

    threshold = max(512, peak // 8)
    start = range_start
    above = np.flatnonzero(np.abs(scan_window) >= threshold)
    if len(above):
        start = range_start + int(above[0]) + sr // 100
    if start < range_start or start >= range_end:
        start = range_start

    window = min(max(sr // 3, 1024), 8192)
    if start + window >= range_end:
        start = range_start
        if window >= range_size:
            window = range_size - 1
    if window < 256:
        return Detection(None, None, None, "rg-nano", "window too short")

    min_lag = max(8, sr // 1200)
    max_lag = sr // 45
    if max_lag >= window // 2:
        max_lag = window // 2
    if max_lag <= min_lag:
        return Detection(None, None, None, "rg-nano", "bad lag range")

    best_lag = 0
    best_corr = 0.0
    for lag in range(min_lag, max_lag + 1):
        corr = rg_correlation(buffer, start, window, lag)
        if corr > best_corr:
            best_corr = corr
            best_lag = lag

    if best_lag <= 0 or best_corr < 0.42:
        return Detection(None, None, best_corr, "rg-nano", "low confidence")

    strong_corr = max(0.42, best_corr - 0.08)
    prev_corr = rg_correlation(buffer, start, window, min_lag)
    for lag in range(min_lag + 1, best_lag):
        corr = rg_correlation(buffer, start, window, lag)
        next_corr = rg_correlation(buffer, start, window, lag + 1)
        if corr >= strong_corr and corr >= prev_corr and corr >= next_corr:
            best_corr = corr
            best_lag = lag
            break
        prev_corr = corr

    freq = float(sr) / float(best_lag)
    return Detection(freq_to_midi(freq), freq, best_corr, "rg-nano")


def detect_librosa(path: Path, start_frame: int = 0, end_frame: int | None = None) -> Detection:
    y, sr = librosa.load(str(path), sr=None, mono=True)
    if len(y) < 256 or sr <= 0:
        return Detection(None, None, None, "desktop-pyin", "too short")
    start = max(0, min(start_frame, len(y) - 1))
    end = len(y) if end_frame is None or end_frame <= start else min(end_frame, len(y))
    y = y[start:end]
    if len(y) < 256:
        return Detection(None, None, None, "desktop-pyin", "trim too short")

    fmin = librosa.note_to_hz("C1")
    fmax = librosa.note_to_hz("C7")
    try:
        f0, voiced_flag, voiced_prob = librosa.pyin(y, fmin=fmin, fmax=fmax, sr=sr)
        voiced = np.isfinite(f0) & voiced_flag
        if np.any(voiced):
            values = f0[voiced]
            probs = voiced_prob[voiced]
            freq = float(np.average(values, weights=np.maximum(probs, 1e-3)))
            return Detection(freq_to_midi(freq), freq, float(np.mean(probs)), "desktop-pyin")
    except Exception as exc:
        pyin_error = str(exc)
    else:
        pyin_error = "unvoiced"

    try:
        f0 = librosa.yin(y, fmin=fmin, fmax=fmax, sr=sr)
        f0 = f0[np.isfinite(f0)]
        if len(f0):
            freq = float(np.median(f0))
            return Detection(freq_to_midi(freq), freq, None, "desktop-yin")
    except Exception as exc:
        return Detection(None, None, None, "desktop-yin", f"{pyin_error}; {exc}")

    return Detection(None, None, None, "desktop-pyin", pyin_error)


def detect_swift_f0(path: Path, start_frame: int = 0, end_frame: int | None = None) -> Detection:
    if SwiftF0 is None:
        return Detection(None, None, None, "desktop-swiftf0", "not installed")
    y, sr = librosa.load(str(path), sr=None, mono=True)
    if len(y) < 256 or sr <= 0:
        return Detection(None, None, None, "desktop-swiftf0", "too short")
    start = max(0, min(start_frame, len(y) - 1))
    end = len(y) if end_frame is None or end_frame <= start else min(end_frame, len(y))
    y = y[start:end]
    if len(y) < 256:
        return Detection(None, None, None, "desktop-swiftf0", "trim too short")
    try:
        detector = SwiftF0(confidence_threshold=0.35, fmin=47.0, fmax=2093.0)
        result = detector.detect_from_array(y.astype(np.float32), sr)
        voiced = result.voicing & np.isfinite(result.pitch_hz) & (result.pitch_hz > 0.0)
        if np.any(voiced):
            freqs = result.pitch_hz[voiced]
            conf = result.confidence[voiced]
            freq = float(np.average(freqs, weights=np.maximum(conf, 1e-3)))
            return Detection(freq_to_midi(freq), freq, float(np.mean(conf)), "desktop-swiftf0")
        return Detection(None, None, None, "desktop-swiftf0", "unvoiced")
    except Exception as exc:
        return Detection(None, None, None, "desktop-swiftf0", str(exc))


def write_sine_fixture(path: Path, midi: int, seconds: float = 1.0, sr: int = 44100) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    freq = midi_to_freq(midi)
    n = int(seconds * sr)
    t = np.arange(n, dtype=np.float64) / sr
    env = np.minimum(1.0, np.arange(n) / 512.0)
    signal = 0.45 * env * np.sin(2.0 * np.pi * freq * t)
    with wave.open(str(path), "wb") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(sr)
        wav.writeframes((np.clip(signal, -1.0, 1.0) * 32767.0).astype("<i2").tobytes())


def write_harmonic_fixture(path: Path, midi: int, seconds: float = 1.0, sr: int = 44100) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    freq = midi_to_freq(midi)
    n = int(seconds * sr)
    t = np.arange(n, dtype=np.float64) / sr
    env = np.minimum(1.0, np.arange(n) / 384.0)
    signal = (
        0.36 * np.sin(2.0 * np.pi * freq * t)
        + 0.16 * np.sin(2.0 * np.pi * 2.0 * freq * t)
        + 0.08 * np.sin(2.0 * np.pi * 3.0 * freq * t)
    )
    signal *= env
    with wave.open(str(path), "wb") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(sr)
        wav.writeframes((np.clip(signal, -1.0, 1.0) * 32767.0).astype("<i2").tobytes())


def write_trim_fixture(path: Path, first_midi: int, second_midi: int, sr: int = 44100) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    n = sr
    chunks = []
    for midi in (first_midi, second_midi):
        freq = midi_to_freq(midi)
        t = np.arange(n, dtype=np.float64) / sr
        env = np.minimum(1.0, np.arange(n) / 512.0)
        chunks.append(0.45 * env * np.sin(2.0 * np.pi * freq * t))
    signal = np.concatenate(chunks)
    with wave.open(str(path), "wb") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(sr)
        wav.writeframes((np.clip(signal, -1.0, 1.0) * 32767.0).astype("<i2").tobytes())


def build_cases(sample_dir: Path, fixture_dir: Path) -> list[Case]:
    cases: list[Case] = []
    for midi in [36, 48, 57, 60, 64, 69, 76, 84]:
        path = fixture_dir / f"sine-{midi}-{note_name(midi)}.wav"
        write_sine_fixture(path, midi)
        cases.append(Case(f"synthetic sine {note_name(midi)}", path, midi))

    for midi in [36, 48, 57, 60, 64, 69, 76, 84]:
        path = fixture_dir / f"harmonic-{midi}-{note_name(midi)}.wav"
        write_harmonic_fixture(path, midi)
        cases.append(Case(f"synthetic harmonic {note_name(midi)}", path, midi))

    trim = fixture_dir / "long-sample-C4-then-A4.wav"
    write_trim_fixture(trim, 60, 69)
    cases.append(Case("long sample whole import", trim, 60, 0, 88200))
    cases.append(Case("long sample trimmed second half", trim, 69, 44100, 88200))

    if sample_dir.exists():
        for path in sorted(sample_dir.rglob("*.wav")):
            cases.append(Case(f"library {path.relative_to(sample_dir)}", path))

    return cases


def run_audit(sample_dir: Path, output_csv: Path | None = None) -> tuple[list[dict[str, str]], dict[str, int]]:
    with tempfile.TemporaryDirectory(prefix="rgnano-root-audit-") as tmp:
        cases = build_cases(sample_dir, Path(tmp))
        rows: list[dict[str, str]] = []
        summary = {
            "synthetic_cases": 0,
            "synthetic_exact": 0,
            "desktop_agreement_cases": 0,
            "desktop_exact": 0,
            "skipped": 0,
        }

        for case in cases:
            try:
                rg = detect_rg_nano(case.path, case.start, case.end)
                swift = detect_swift_f0(case.path, case.start, case.end)
                desk = detect_librosa(case.path, case.start, case.end)
            except Exception as exc:
                summary["skipped"] += 1
                rows.append(
                    {
                        "status": "skip",
                        "case": case.label,
                        "file": str(case.path),
                        "start": str(case.start),
                        "end": str(case.end or ""),
                        "expected": note_name(case.expected_midi),
                        "desktop": "--",
                        "desktop_method": "unreadable",
                        "swiftf0": "--",
                        "pyin_yin": "--",
                        "rg_nano": "--",
                        "rg_confidence": "",
                        "delta_cents": "",
                        "skip_reason": str(exc),
                    }
                )
                continue
            reference = swift if swift.midi is not None else desk
            target = case.expected_midi if case.expected_midi is not None else reference.midi
            delta = cents_between(rg.midi, target)
            status = "skip" if rg.midi is None else "ok"
            if case.expected_midi is not None:
                summary["synthetic_cases"] += 1
                if rg.midi == case.expected_midi:
                    summary["synthetic_exact"] += 1
                else:
                    status = "mismatch"
            elif reference.midi is not None and rg.midi is not None:
                summary["desktop_agreement_cases"] += 1
                if abs(rg.midi - reference.midi) <= 1:
                    summary["desktop_exact"] += 1
                else:
                    status = "review"
            elif rg.midi is None:
                summary["skipped"] += 1

            rows.append(
                {
                    "status": status,
                    "case": case.label,
                    "file": str(case.path),
                    "start": str(case.start),
                    "end": str(case.end or ""),
                    "expected": note_name(case.expected_midi),
                    "desktop": note_name(reference.midi),
                    "desktop_method": reference.method,
                    "swiftf0": note_name(swift.midi),
                    "pyin_yin": note_name(desk.midi),
                    "rg_nano": note_name(rg.midi),
                    "rg_confidence": "" if rg.confidence is None else f"{rg.confidence:.3f}",
                    "delta_cents": "" if delta is None else str(delta),
                    "skip_reason": rg.skipped or reference.skipped or desk.skipped,
                }
            )

    if output_csv:
        output_csv.parent.mkdir(parents=True, exist_ok=True)
        with output_csv.open("w", newline="", encoding="utf-8") as fh:
            writer = csv.DictWriter(fh, fieldnames=list(rows[0].keys()))
            writer.writeheader()
            writer.writerows(rows)

    return rows, summary


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--sample-dir", type=Path, default=DEFAULT_SAMPLE_DIR)
    parser.add_argument("--csv", type=Path, default=ROOT / "exports" / "root-detection-accuracy.csv")
    parser.add_argument("--show-all", action="store_true")
    args = parser.parse_args()

    rows, summary = run_audit(args.sample_dir, args.csv)
    print("RG Nano root detection accuracy audit")
    print(f"sample dir: {args.sample_dir}")
    print(
        "synthetic exact: "
        f"{summary['synthetic_exact']}/{summary['synthetic_cases']} "
        f"({summary['synthetic_exact'] / max(1, summary['synthetic_cases']) * 100:.1f}%)"
    )
    print(
        "desktop agreement within 1 semitone: "
        f"{summary['desktop_exact']}/{summary['desktop_agreement_cases']} "
        f"({summary['desktop_exact'] / max(1, summary['desktop_agreement_cases']) * 100:.1f}%)"
    )
    print(f"skipped/manual cases: {summary['skipped']}")
    print(f"csv: {args.csv}")

    interesting = rows if args.show_all else [row for row in rows if row["status"] in {"mismatch", "review", "skip"}]
    if interesting:
        print()
        print("cases to review:")
        for row in interesting[:40]:
            print(
                f"- {row['status']}: {row['case']} "
                f"desktop={row['desktop']} rg={row['rg_nano']} "
                f"expected={row['expected']} {row['skip_reason']}"
            )

    return 0 if summary["synthetic_exact"] == summary["synthetic_cases"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
