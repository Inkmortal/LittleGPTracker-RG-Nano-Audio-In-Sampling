#!/usr/bin/env python3
"""Self-test the RG Nano layout audit heuristics."""

from __future__ import annotations

import tempfile
from pathlib import Path

from PIL import Image, ImageDraw

import rgnano_layout_audit


def write_bad_source(path: Path) -> None:
    img = Image.new("RGB", (240, 240), (24, 6, 27))
    draw = ImageDraw.Draw(img)
    magenta = (219, 51, 219)
    white = (245, 235, 255)

    # Deliberately too tall and too low: this mimics the earlier mashed
    # Source/Loop failure where the waveform ate the legend and field area.
    draw.rectangle((8, 28, 232, 128), outline=magenta)
    for x in range(10, 230):
        y = 78 + ((x * 7) % 34) - 17
        draw.line((x, 78, x, y), fill=magenta if x % 5 else white)

    img.save(path)


def main() -> int:
    with tempfile.TemporaryDirectory() as temp:
        path = Path(temp) / "instrument-lab-source.png"
        write_bad_source(path)
        issues = rgnano_layout_audit.audit_paths([str(path)])
        if not issues:
            print("layout audit self-test failed: bad source screen passed")
            return 1
        print("layout audit self-test passed:")
        for issue in issues:
            print(f"- {issue.message}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
