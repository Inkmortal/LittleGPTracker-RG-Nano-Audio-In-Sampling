#!/usr/bin/env python3
"""Programmatic RG Nano screenshot layout audit.

This is intentionally conservative. It does not try to understand whether a
screen is beautiful; it catches the tiny-screen failures that are easy to miss
by eye: wrong dimensions, clipped bright pixels on the edge, text/control rows
mashed together, and page-specific visual panels colliding with editable fields.
"""

from __future__ import annotations

import argparse
import glob
import os
import sys
from collections import deque
from dataclasses import dataclass
from typing import Iterable, List, Optional, Sequence, Tuple

from PIL import Image


Color = Tuple[int, int, int]
BBox = Tuple[int, int, int, int]


@dataclass
class Issue:
    path: str
    message: str


@dataclass
class Component:
    bbox: BBox
    pixels: int

    @property
    def width(self) -> int:
        return self.bbox[2] - self.bbox[0] + 1

    @property
    def height(self) -> int:
        return self.bbox[3] - self.bbox[1] + 1


def dist(a: Color, b: Color) -> int:
    return abs(a[0] - b[0]) + abs(a[1] - b[1]) + abs(a[2] - b[2])


def dominant_color(pixels: Sequence[Color]) -> Color:
    counts = {}
    for p in pixels:
        counts[p] = counts.get(p, 0) + 1
    return max(counts.items(), key=lambda item: item[1])[0]


def load_rgb(path: str) -> Image.Image:
    return Image.open(path).convert("RGB")


def foreground_mask(img: Image.Image, threshold: int = 34) -> List[List[bool]]:
    width, height = img.size
    get_pixels = getattr(img, "get_flattened_data", img.getdata)
    pixels = list(get_pixels())
    bg = dominant_color(pixels)
    mask: List[List[bool]] = []
    for y in range(height):
        row = []
        for x in range(width):
            row.append(dist(pixels[y * width + x], bg) >= threshold)
        mask.append(row)
    return mask


def components(mask: List[List[bool]], min_pixels: int = 20) -> List[Component]:
    height = len(mask)
    width = len(mask[0]) if height else 0
    seen = [[False] * width for _ in range(height)]
    found: List[Component] = []
    for y in range(height):
        for x in range(width):
            if seen[y][x] or not mask[y][x]:
                continue
            q = deque([(x, y)])
            seen[y][x] = True
            count = 0
            min_x = max_x = x
            min_y = max_y = y
            while q:
                cx, cy = q.popleft()
                count += 1
                min_x = min(min_x, cx)
                max_x = max(max_x, cx)
                min_y = min(min_y, cy)
                max_y = max(max_y, cy)
                for nx, ny in ((cx + 1, cy), (cx - 1, cy), (cx, cy + 1), (cx, cy - 1)):
                    if nx < 0 or ny < 0 or nx >= width or ny >= height:
                        continue
                    if seen[ny][nx] or not mask[ny][nx]:
                        continue
                    seen[ny][nx] = True
                    q.append((nx, ny))
            if count >= min_pixels:
                found.append(Component((min_x, min_y, max_x, max_y), count))
    return found


def row_ink(mask: List[List[bool]], row: int) -> int:
    y0 = row * 8
    y1 = min(y0 + 8, len(mask))
    if y0 >= y1:
        return 0
    return sum(1 for y in range(y0, y1) for x in range(len(mask[y])) if mask[y][x])


def row_active_cells(mask: List[List[bool]], row: int) -> int:
    y0 = row * 8
    y1 = min(y0 + 8, len(mask))
    width = len(mask[0])
    cells = 0
    for cell_x in range(width // 8):
        x0 = cell_x * 8
        x1 = x0 + 8
        ink = sum(1 for y in range(y0, y1) for x in range(x0, x1) if mask[y][x])
        if ink >= 4:
            cells += 1
    return cells


def rect_ink(mask: List[List[bool]], rect: BBox) -> int:
    x0, y0, x1, y1 = rect
    x0 = max(0, x0)
    y0 = max(0, y0)
    x1 = min(len(mask[0]) - 1, x1)
    y1 = min(len(mask) - 1, y1)
    return sum(1 for y in range(y0, y1 + 1) for x in range(x0, x1 + 1) if mask[y][x])


def audit_generic(path: str, img: Image.Image, mask: List[List[bool]]) -> List[Issue]:
    issues: List[Issue] = []
    name = os.path.basename(path).lower()
    is_sample_lab = "instrument-lab-" in name or "sample-lab-" in name
    if img.size != (240, 240):
        issues.append(Issue(path, f"expected 240x240 app screenshot, got {img.size[0]}x{img.size[1]}"))

    # Bright/colored content on the outermost app edge usually means clipping.
    edge_ink = (
        rect_ink(mask, (0, 0, 239, 0))
        + rect_ink(mask, (0, 239, 239, 239))
        + rect_ink(mask, (0, 0, 0, 239))
        + rect_ink(mask, (239, 0, 239, 239))
    )
    if edge_ink > 24:
        issues.append(Issue(path, f"too much foreground on app edge ({edge_ink}px); likely clipped or bleeding"))

    for row in range(30):
        if is_sample_lab and 4 <= row <= 11:
            # Dense waveform/meter rows are expected inside the sample lab
            # visual panel. Screen-specific checks below verify that the panel
            # stays out of the editable field area.
            continue
        active = row_active_cells(mask, row)
        ink = row_ink(mask, row)
        if active >= 28 and ink > 900:
            issues.append(Issue(path, f"row {row} is nearly full and dense ({active} cells, {ink}px); likely unreadable"))

    return issues


def largest_visual_component(mask: List[List[bool]], min_width: int = 80,
                             max_top: int = 120) -> Optional[Component]:
    comps = [
        c for c in components(mask, min_pixels=80)
        if c.width >= min_width and c.height >= 10 and c.bbox[1] <= max_top
    ]
    if not comps:
        return None
    return max(comps, key=lambda c: c.pixels)


def audit_sample_lab(path: str, mask: List[List[bool]]) -> List[Issue]:
    issues: List[Issue] = []
    name = os.path.basename(path).lower()
    if "instrument-lab-" not in name and "sample-lab-" not in name:
        return issues

    # Rows 0-3 are title/status. Visual controls should live below that.
    comp = largest_visual_component(mask, min_width=8 if "motion" in name else 80)
    if comp is None:
        issues.append(Issue(path, "no substantial visual/control component detected"))
        return issues

    x0, y0, x1, y1 = comp.bbox
    if "source" in name or "loop" in name:
        if y0 < 32:
            issues.append(Issue(path, f"waveform starts too high at y={y0}; collides with title/status area"))
        if y1 > 96:
            issues.append(Issue(path, f"waveform ends too low at y={y1}; collides with legend/field area"))

        legend_ink = row_ink(mask, 12)
        sample_or_value_ink = row_ink(mask, 14)
        first_field_ink = row_ink(mask, 17)
        spacer_ink = row_ink(mask, 16)
        if legend_ink < 40:
            issues.append(Issue(path, "missing/too faint marker legend row"))
        if sample_or_value_ink < 40:
            issues.append(Issue(path, "missing sample name or loop value summary row"))
        if first_field_ink < 40:
            issues.append(Issue(path, "editable field rows did not start where expected"))
        if spacer_ink > 120:
            issues.append(Issue(path, f"spacer row before editable fields is crowded ({spacer_ink}px)"))

    if "shape" in name or "filter" in name:
        # Three horizontal controls should fit above row 13 and leave the field
        # list below row 17 readable.
        if y0 < 38:
            issues.append(Issue(path, f"control panel starts too high at y={y0}"))
        if y1 > 106:
            issues.append(Issue(path, f"control panel ends too low at y={y1}; field rows may be mashed"))
        if row_ink(mask, 14) < 40:
            issues.append(Issue(path, "missing parameter summary row above editable fields"))
        if row_ink(mask, 16) > 120:
            issues.append(Issue(path, "crowded spacer row before editable fields"))

    if "motion" in name:
        if y1 > 110:
            issues.append(Issue(path, f"motion visualization ends too low at y={y1}; field rows may be mashed"))
        if row_ink(mask, 13) < 40:
            issues.append(Issue(path, "missing table automation summary row"))

    return issues


def audit_paths(paths: Iterable[str]) -> List[Issue]:
    issues: List[Issue] = []
    for path in paths:
        img = load_rgb(path)
        mask = foreground_mask(img)
        issues.extend(audit_generic(path, img, mask))
        issues.extend(audit_sample_lab(path, mask))
    return issues


def expand_inputs(inputs: Sequence[str]) -> List[str]:
    paths: List[str] = []
    for item in inputs:
        matches = glob.glob(item)
        if matches:
            paths.extend(matches)
        elif os.path.exists(item):
            paths.append(item)
        else:
            raise FileNotFoundError(item)
    return sorted(set(paths))


def main(argv: Optional[Sequence[str]] = None) -> int:
    parser = argparse.ArgumentParser(description="Audit RG Nano app screenshots for cramped/unreadable layout.")
    parser.add_argument("paths", nargs="+", help="BMP/PNG paths or globs to audit")
    args = parser.parse_args(argv)

    try:
        paths = expand_inputs(args.paths)
    except FileNotFoundError as exc:
        print(f"layout audit: missing input {exc}", file=sys.stderr)
        return 2

    issues = audit_paths(paths)
    if issues:
        print("RG Nano layout audit failed:")
        for issue in issues:
            print(f"- {issue.path}: {issue.message}")
        return 1

    print(f"RG Nano layout audit passed ({len(paths)} screenshot(s)).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
