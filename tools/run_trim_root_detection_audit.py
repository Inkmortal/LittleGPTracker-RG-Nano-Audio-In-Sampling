#!/usr/bin/env python3
"""Build and run the trim-aware root detection simulator check."""

from __future__ import annotations

import math
import os
import struct
import subprocess
import sys
import wave
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
PROJECTS = ROOT / "projects"
SAMPLES = PROJECTS / "rgnano-sim-data" / "samples"
FIXTURE = SAMPLES / "pitch-window-fixture.wav"
STALE_FIXTURE = SAMPLES / "trim-root-fixture.wav"


def msys_path(path: Path) -> str:
    resolved = path.resolve()
    drive = resolved.drive.rstrip(":").lower()
    return "/" + drive + "/" + "/".join(resolved.parts[1:]).replace("\\", "/")


def write_fixture(path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    sample_rate = 44100
    notes = [(261.625565, sample_rate), (440.0, sample_rate)]
    with wave.open(str(path), "wb") as wav:
        wav.setnchannels(1)
        wav.setsampwidth(2)
        wav.setframerate(sample_rate)
        for freq, frames in notes:
            for i in range(frames):
                env = min(1.0, i / 512.0)
                value = int(16000 * env * math.sin(2.0 * math.pi * freq * i / sample_rate))
                wav.writeframesraw(struct.pack("<h", value))


def cleanup_fixture() -> None:
    for path in (FIXTURE, STALE_FIXTURE):
        try:
            if path.exists():
                path.unlink()
        except OSError:
            pass


def run(cmd: list[str], cwd: Path, env: dict[str, str]) -> None:
    print("+", " ".join(cmd))
    subprocess.run(cmd, cwd=str(cwd), env=env, check=True)


def main() -> int:
    make = Path("C:/msys64/usr/bin/make.exe")
    if not make.exists():
        print(f"missing MSYS make: {make}", file=sys.stderr)
        return 2

    cleanup_fixture()
    write_fixture(FIXTURE)

    env = os.environ.copy()
    env["PATH"] = f"C:/msys64/mingw32/bin;C:/msys64/usr/bin;{env.get('PATH', '')}"
    env["PWD"] = msys_path(PROJECTS)
    try:
        run([str(make), "PLATFORM=RGNANO_SIM", "CXX=C:/msys64/mingw32/bin/g++.exe"], PROJECTS, env)

        sim_env = env.copy()
        sim_env["SDL_VIDEODRIVER"] = "dummy"
        run([
            str(PROJECTS / "lgpt-rgnano-sim.exe"),
            "-AUTO_LOAD_LAST=NO",
            "-RGNANOSIM_SCRIPT=resources\\RGNANO_SIM\\trim-root-detection-workflow.rgsim",
        ], PROJECTS, sim_env)
    finally:
        cleanup_fixture()

    print("Trim root detection audit passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
