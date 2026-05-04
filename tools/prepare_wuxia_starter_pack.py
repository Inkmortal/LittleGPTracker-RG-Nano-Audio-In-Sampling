#!/usr/bin/env python3
"""Build a small Chinese-instrument starter sample pack for RG Nano testing.

The pack uses public Wikimedia Commons source audio, converts each source to a
small mono WAV that LGPT can import comfortably, analyzes the resulting waveform,
and mirrors the WAV files into the RG Nano simulator sample library.
"""

from __future__ import annotations

import json
import math
import shutil
import struct
import subprocess
import sys
import time
import urllib.parse
import urllib.request
import wave
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
EXPORT_ROOT = ROOT / "exports" / "rg-nano-sample-packs" / "wuxia-starter"
SOURCE_DIR = EXPORT_ROOT / "source"
SAMPLES_DIR = EXPORT_ROOT / "Samples"
WAVEFORM_DIR = EXPORT_ROOT / "waveforms"
SIM_SAMPLES_DIR = ROOT / "projects" / "rgnano-sim-data" / "samples" / "wuxia-starter"


@dataclass(frozen=True)
class SampleSource:
    slug: str
    file_name: str
    label: str
    instrument: str
    seconds: float
    start: float = 0.0
    gain_db: float = -1.0

    @property
    def page_url(self) -> str:
        return "https://commons.wikimedia.org/wiki/File:" + urllib.parse.quote(self.file_name)

    @property
    def download_url(self) -> str:
        return "https://commons.wikimedia.org/wiki/Special:Redirect/file/" + urllib.parse.quote(self.file_name)


SOURCES = [
    SampleSource("dizi-flute", "DiZi Chinese Flute Sample.ogg", "Dizi flute phrase", "dizi", 8.0),
    SampleSource("pipa-pluck", "Pipa - sound.ogg", "Pipa plucked phrase", "pipa", 8.0),
    SampleSource("banhu-bow", "Banhu.ogg", "Banhu bowed phrase", "banhu", 8.0),
    SampleSource("suona-horn", "Suona.ogg", "Suona horn phrase", "suona", 8.0),
    SampleSource("zhongruan-pluck", "Zhongruan.ogg", "Zhongruan plucked phrase", "zhongruan", 8.0),
    SampleSource("erhu-moon", "二泉映月.ogg", "Erhu melodic phrase", "erhu", 8.0, 0.0, -3.0),
    SampleSource("play-pipa-hit", "Pipa - sound.ogg", "Playable pipa hit", "pipa", 0.85, 0.0, 3.0),
    SampleSource("play-zhongruan-hit", "Zhongruan.ogg", "Playable zhongruan hit", "zhongruan", 0.95, 0.0, 12.0),
    SampleSource("play-dizi-note", "DiZi Chinese Flute Sample.ogg", "Playable dizi note", "dizi", 1.5, 1.2, 1.0),
    SampleSource("play-erhu-swell", "二泉映月.ogg", "Playable erhu swell", "erhu", 1.8, 2.4, 1.0),
    SampleSource("play-banhu-saw", "Banhu.ogg", "Playable banhu bow", "banhu", 1.2, 1.0, 7.0),
    SampleSource("play-suona-accent", "Suona.ogg", "Playable suona accent", "suona", 1.2, 0.2, -1.0),
]


def ensure_dirs() -> None:
    for path in (SOURCE_DIR, SAMPLES_DIR, WAVEFORM_DIR, SIM_SAMPLES_DIR):
        path.mkdir(parents=True, exist_ok=True)


def download(source: SampleSource) -> Path:
    dest = SOURCE_DIR / source.file_name
    if dest.exists() and dest.stat().st_size > 0:
        return dest
    request = urllib.request.Request(
        source.download_url,
        headers={"User-Agent": "LittleGPTracker-RG-Nano-sample-pack-builder/1.0"},
    )
    last_error: Exception | None = None
    for attempt in range(5):
        try:
            with urllib.request.urlopen(request, timeout=60) as response:
                data = response.read()
            break
        except Exception as exc:
            last_error = exc
            sleep_seconds = 3 * (attempt + 1)
            print(f"  download retry {attempt + 1}/5 after {sleep_seconds}s: {exc}")
            time.sleep(sleep_seconds)
    else:
        raise RuntimeError(f"download failed for {source.file_name}: {last_error}")
    if len(data) < 1024:
        raise RuntimeError(f"download too small for {source.file_name}: {len(data)} bytes")
    dest.write_bytes(data)
    time.sleep(2)
    return dest


def convert(source: SampleSource, source_path: Path) -> Path:
    dest = SAMPLES_DIR / f"wx_{source.slug}.wav"
    cmd = [
        "ffmpeg",
        "-y",
        "-hide_banner",
        "-loglevel",
        "error",
        "-i",
        str(source_path),
        "-ss",
        str(source.start),
        "-t",
        str(source.seconds),
        "-ac",
        "1",
        "-ar",
        "22050",
        "-sample_fmt",
        "s16",
        "-af",
        f"volume={source.gain_db}dB,afade=t=in:st=0:d=0.02,afade=t=out:st={max(source.seconds - 0.05, 0):.2f}:d=0.05",
        str(dest),
    ]
    subprocess.run(cmd, cwd=str(ROOT), check=True)
    shutil.copy2(dest, SIM_SAMPLES_DIR / dest.name)
    return dest


def read_samples(path: Path) -> tuple[int, list[int]]:
    with wave.open(str(path), "rb") as wav:
        channels = wav.getnchannels()
        sample_rate = wav.getframerate()
        sample_width = wav.getsampwidth()
        frames = wav.readframes(wav.getnframes())
    if channels != 1 or sample_width != 2:
        raise RuntimeError(f"expected mono 16-bit WAV: {path}")
    count = len(frames) // 2
    samples = list(struct.unpack("<" + "h" * count, frames))
    return sample_rate, samples


def zero_crossing_rate(samples: list[int]) -> float:
    if len(samples) < 2:
        return 0.0
    changes = 0
    prev = samples[0]
    for sample in samples[1:]:
        if (prev < 0 <= sample) or (prev >= 0 > sample):
            changes += 1
        prev = sample
    return changes / float(len(samples) - 1)


def envelope(samples: list[int], buckets: int = 64) -> list[float]:
    if not samples:
        return [0.0] * buckets
    out = []
    step = max(1, len(samples) // buckets)
    for i in range(buckets):
        chunk = samples[i * step:(i + 1) * step]
        if not chunk:
            out.append(0.0)
        else:
            out.append(max(abs(v) for v in chunk) / 32768.0)
    return out


def waveform_text(values: list[float]) -> str:
    chars = " .:-=+*#%@"
    lines = []
    height = 12
    for row in range(height, -1, -1):
        threshold = row / float(height)
        line = "".join(chars[min(len(chars) - 1, int(v * (len(chars) - 1)))] if v >= threshold else " " for v in values)
        lines.append(line.rstrip())
    return "\n".join(lines)


def analyze(path: Path) -> dict[str, object]:
    sample_rate, samples = read_samples(path)
    peak = max(abs(v) for v in samples) if samples else 0
    rms = math.sqrt(sum(float(v) * float(v) for v in samples) / max(1, len(samples)))
    env = envelope(samples)
    text = waveform_text(env)
    wave_path = WAVEFORM_DIR / (path.stem + ".txt")
    wave_path.write_text(text + "\n", encoding="utf-8")
    return {
        "file": path.name,
        "sample_rate": sample_rate,
        "channels": 1,
        "bits": 16,
        "duration_seconds": round(len(samples) / float(sample_rate), 3),
        "peak": round(peak / 32768.0, 4),
        "rms": round(rms / 32768.0, 4),
        "zero_crossing_rate": round(zero_crossing_rate(samples), 5),
        "waveform_text": str(wave_path.relative_to(EXPORT_ROOT)).replace("\\", "/"),
    }


def write_readme(records: list[dict[str, object]]) -> None:
    lines = [
        "# Wuxia Starter Sample Pack",
        "",
        "Small Chinese-instrument WAV pack prepared for RG Nano / RGNANO_SIM testing.",
        "",
        "Copy the contents of `Samples/` to the RG Nano SD card folder:",
        "",
        "```text",
        "/mnt/Samples/wuxia-starter/",
        "```",
        "",
        "The same WAVs are mirrored into:",
        "",
        "```text",
        "projects/rgnano-sim-data/samples/wuxia-starter/",
        "```",
        "",
        "Files are converted to mono 16-bit 22050 Hz WAV and trimmed to short phrases so they stay friendly to the tiny device.",
        "",
        "## Samples",
        "",
        "| File | Instrument | Source | Peak | RMS |",
        "| --- | --- | --- | --- | --- |",
    ]
    for record in records:
        source = record["source"]
        lines.append(
            f"| `{record['file']}` | {record['instrument']} | [Wikimedia Commons]({source}) | {record['peak']} | {record['rms']} |"
        )
    lines.extend([
        "",
        "Check each Wikimedia file page before commercial release. These are meant as a convenient test pack, not a replacement for a curated paid sample library.",
    ])
    (EXPORT_ROOT / "README.md").write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    ensure_dirs()
    records = []
    for source in SOURCES:
        print(f"download/convert/analyze: {source.label}")
        raw = download(source)
        wav = convert(source, raw)
        analysis = analyze(wav)
        analysis.update({
            "label": source.label,
            "instrument": source.instrument,
            "source": source.page_url,
            "download": source.download_url,
        })
        records.append(analysis)
    (EXPORT_ROOT / "analysis.json").write_text(json.dumps(records, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    write_readme(records)
    print(f"wrote {EXPORT_ROOT}")
    print(f"mirrored WAVs to {SIM_SAMPLES_DIR}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
