#!/usr/bin/env python3
"""Run the RG Nano sample-lab simulator capture plus layout audit.

This avoids the PowerShell helper path for the user's device/workflow. It is a
plain Python runner that:

1. builds the RG Nano simulator with MSYS make,
2. runs the sample lab preview script using SDL's dummy video driver,
3. audits the generated app screenshots for cramped/unreadable layout, and
4. removes generated screenshots/logs/temp projects when the run is done.
"""

from __future__ import annotations

import os
import shutil
import subprocess
import sys
from pathlib import Path

import rgnano_layout_audit


ROOT = Path(__file__).resolve().parents[1]
PROJECTS = ROOT / "projects"
TRACKS = PROJECTS / "rgnano-sim-data" / "tracks"
KEEP_TRACKS = {"lgpt_DopeyBite", "lgpt_MildCook", "lgpt_SingingSnap"}
SCREENSHOTS = [
    PROJECTS / "sample-lab-source.bmp",
    PROJECTS / "sample-lab-shape.bmp",
    PROJECTS / "sample-lab-filter.bmp",
    PROJECTS / "sample-lab-loop.bmp",
    PROJECTS / "sample-lab-motion.bmp",
]


def msys_path(path: Path) -> str:
    resolved = path.resolve()
    drive = resolved.drive.rstrip(":").lower()
    parts = resolved.parts[1:]
    return "/" + drive + "/" + "/".join(parts).replace("\\", "/")


def cleanup() -> None:
    for screenshot in SCREENSHOTS:
        if screenshot.exists():
            screenshot.unlink()

    for path in (PROJECTS / "last_project", PROJECTS / "rgnano-sim.log"):
        if path.is_dir():
            shutil.rmtree(path)
        elif path.exists():
            path.unlink()

    if TRACKS.exists():
        for child in TRACKS.glob("lgpt_*"):
            if child.is_dir() and child.name not in KEEP_TRACKS:
                shutil.rmtree(child)


def run(cmd: list[str], cwd: Path, env: dict[str, str]) -> None:
    print("+", " ".join(cmd))
    subprocess.run(cmd, cwd=str(cwd), env=env, check=True)


def main() -> int:
    make = Path("C:/msys64/usr/bin/make.exe")
    mingw_bin = Path("C:/msys64/mingw32/bin")
    msys_bin = Path("C:/msys64/usr/bin")
    if not make.exists():
        print(f"missing MSYS make: {make}", file=sys.stderr)
        return 2

    cleanup()

    env = os.environ.copy()
    env["PATH"] = f"{mingw_bin};{msys_bin};{env.get('PATH', '')}"
    env["PWD"] = msys_path(PROJECTS)
    run([str(make), "PLATFORM=RGNANO_SIM"], PROJECTS, env)

    sim_env = env.copy()
    sim_env["SDL_VIDEODRIVER"] = "dummy"
    run([
        str(PROJECTS / "lgpt-rgnano-sim.exe"),
        "-AUTO_LOAD_LAST=NO",
        "-RGNANOSIM_SCRIPT=resources\\RGNANO_SIM\\sample-lab-pages-preview.rgsim",
    ], PROJECTS, sim_env)

    issues = rgnano_layout_audit.audit_paths(str(path) for path in SCREENSHOTS)
    if issues:
        print("RG Nano sample lab layout audit failed:")
        for issue in issues:
            print(f"- {issue.path}: {issue.message}")
        return 1

    print(f"RG Nano sample lab layout audit passed ({len(SCREENSHOTS)} screenshot(s)).")
    cleanup()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
