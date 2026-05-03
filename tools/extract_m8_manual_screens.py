#!/usr/bin/env python3
"""Extract M8 manual screen references as rendered pixels plus text geometry.

The Dirtywave manual draws its reference screens as PDF vectors/text, not as
embedded bitmap screenshots. This script finds the large stroked screen
rectangles on the documented view pages, renders each rectangle to PNG, and
emits a manifest with manual/PDF page references plus the text glyph positions
inside each crop.
"""

from __future__ import annotations

import argparse
import json
import urllib.request
from dataclasses import dataclass
from pathlib import Path
from typing import Any

import fitz
from PIL import Image


MANUAL_URL = "https://cdn.shopify.com/s/files/1/0455/0485/6229/files/m8_operation_manual_v20260421.pdf?v=1776791699"


@dataclass(frozen=True)
class ScreenRef:
    key: str
    label: str
    manual_page: int
    end_page: int | None = None
    rect_pick: str = "largest"


SCREEN_REFS = [
    ScreenRef("song", "Song View", 10),
    ScreenRef("live", "Live Mode", 10),
    ScreenRef("chain", "Chain View", 12),
    ScreenRef("phrase", "Phrase View", 14),
    ScreenRef("instrument", "Instrument View", 16),
    ScreenRef("modulation", "Instrument Modulation View", 18),
    ScreenRef("pool", "Instrument Pool View", 22),
    ScreenRef("table", "Table View", 24),
    ScreenRef("groove", "Groove View", 26),
    ScreenRef("scale", "Scale View", 28),
    ScreenRef("mixer", "Mixer View", 30),
    ScreenRef("eq", "EQ Editor View", 32),
    ScreenRef("limiter", "Limiter & Mix Scope View", 34),
    ScreenRef("fx", "Effect Settings View", 36),
    ScreenRef("project", "Project View", 38),
    ScreenRef("system", "System Settings View", 40),
    ScreenRef("theme", "Theme View", 42),
    ScreenRef("midi_mapping", "MIDI Mapping View", 43),
    ScreenRef("midi", "MIDI Settings View", 44),
    ScreenRef("render", "Render View", 47),
    ScreenRef("command", "Effect Command Help View", 48, rect_pick="lowest"),
    ScreenRef("sampler", "Sampler", 54),
    ScreenRef("sample_editor", "Sample Editor", 56),
]


def pdf_page_number(manual_page: int) -> int:
    """Return the one-based PDF viewer page for a printed manual page."""
    return manual_page + 7


def page_index(manual_page: int) -> int:
    """Return the zero-based PyMuPDF page index for a printed manual page."""
    return manual_page + 6


def ensure_pdf(path: Path) -> None:
    if path.exists() and path.stat().st_size > 0:
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    with urllib.request.urlopen(MANUAL_URL, timeout=60) as response:
        path.write_bytes(response.read())


def screen_rect_candidates(page: fitz.Page) -> list[fitz.Rect]:
    candidates: list[fitz.Rect] = []
    for drawing in page.get_drawings():
        rect = drawing.get("rect")
        if rect is None:
            continue
        if drawing.get("type") != "s":
            continue
        if rect.width < 80 or rect.height < 50:
            continue
        candidates.append(fitz.Rect(rect))
    return sorted(candidates, key=lambda rect: (rect.y0, rect.x0))


def choose_rect(candidates: list[fitz.Rect], pick: str) -> fitz.Rect:
    if not candidates:
        raise ValueError("No candidate screen rectangles found")
    if pick == "lowest":
        return max(candidates, key=lambda rect: (rect.y0, rect.width * rect.height))
    if pick == "leftmost":
        return min(candidates, key=lambda rect: (rect.x0, -rect.width * rect.height))
    return max(candidates, key=lambda rect: rect.width * rect.height)


def relative_bbox(bbox: tuple[float, float, float, float], rect: fitz.Rect) -> list[float]:
    return [
        round(bbox[0] - rect.x0, 3),
        round(bbox[1] - rect.y0, 3),
        round(bbox[2] - rect.x0, 3),
        round(bbox[3] - rect.y0, 3),
    ]


def extract_text_geometry(page: fitz.Page, rect: fitz.Rect) -> dict[str, Any]:
    """Extract selectable PDF text.

    The surrounding manual prose is selectable text, but the reference screens
    are mostly vector outlines. For those screens this intentionally returns an
    empty char list while the drawing extractor below captures the visual
    primitives that make up the glyphs.
    """
    raw = page.get_text("rawdict", clip=rect)
    spans: list[dict[str, Any]] = []
    chars: list[dict[str, Any]] = []

    for block in raw.get("blocks", []):
        for line in block.get("lines", []):
            for span in line.get("spans", []):
                text = "".join(ch.get("c", "") for ch in span.get("chars", []))
                if text:
                    spans.append({
                        "text": text,
                        "bbox": relative_bbox(tuple(span["bbox"]), rect),
                        "size": round(span.get("size", 0), 3),
                        "font": span.get("font", ""),
                        "color": span.get("color"),
                    })
                for ch in span.get("chars", []):
                    value = ch.get("c", "")
                    if value:
                        chars.append({
                            "c": value,
                            "bbox": relative_bbox(tuple(ch["bbox"]), rect),
                            "color": span.get("color"),
                            "font": span.get("font", ""),
                            "size": round(span.get("size", 0), 3),
                        })

    return {"spans": spans, "chars": chars}


def extract_drawings(page: fitz.Page, rect: fitz.Rect) -> list[dict[str, Any]]:
    drawings: list[dict[str, Any]] = []
    for drawing in page.get_drawings():
        drect = drawing.get("rect")
        if drect is None or not fitz.Rect(drect).intersects(rect):
            continue
        drawings.append({
            "type": drawing.get("type"),
            "bbox": relative_bbox(tuple(drect), rect),
            "fill": drawing.get("fill"),
            "color": drawing.get("color"),
            "items": len(drawing.get("items", [])),
        })
    return drawings


def render_crop(page: fitz.Page, rect: fitz.Rect, scale: float, out_path: Path) -> dict[str, int]:
    matrix = fitz.Matrix(scale, scale)
    pix = page.get_pixmap(matrix=matrix, clip=rect, alpha=False)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    pix.save(out_path)
    return {"width": pix.width, "height": pix.height}


def stylize_manual_crop(src_path: Path, out_path: Path) -> None:
    """Turn the white manual crop into a dark M8-style display image.

    This intentionally preserves the manual crop's exact layout and glyph shapes.
    Only the palette changes: paper becomes a dark display, black/gray glyphs
    become light/muted display pixels, and the M8 accent colors get boosted.
    """
    image = Image.open(src_path).convert("RGBA")
    pixels = image.load()
    width, height = image.size

    dark_bg = (2, 3, 10, 255)
    dark_bg_2 = (5, 7, 18, 255)
    lavender = (211, 194, 255, 255)
    lavender_dim = (133, 122, 164, 255)
    faint = (61, 61, 83, 255)
    purple = (206, 78, 255, 255)
    blue = (60, 184, 255, 255)

    for y in range(height):
        shade = dark_bg if y < height * 0.55 else dark_bg_2
        for x in range(width):
            r, g, b, a = pixels[x, y]
            if a == 0:
                continue
            mx = max(r, g, b)
            mn = min(r, g, b)
            sat = mx - mn

            if r > 238 and g > 238 and b > 238:
                pixels[x, y] = shade
            elif sat < 42 and mx < 96:
                pixels[x, y] = lavender
            elif sat < 42 and mx < 205:
                # Keep neutral meter fills neutral. Earlier versions shifted
                # these into lavender, which made the mixer bars look wrong.
                lift = 1.02 if mx < 150 else 1.0
                pixels[x, y] = (
                    min(255, int(r * lift)),
                    min(255, int(g * lift)),
                    min(255, int(b * lift)),
                    a,
                )
            elif sat < 42:
                pixels[x, y] = faint
            elif r > 170 and b > 105 and g < 95:
                pixels[x, y] = purple
            elif b > 145 and g > 110 and r < 90:
                pixels[x, y] = blue
            elif g > 170 and r > 95 and b > 65:
                # Preserve extracted meter/bar green-yellow/teal gradients.
                pixels[x, y] = (r, min(255, int(g * 1.08)), b, a)
            elif g > 130 and r < 120 and b < 130:
                pixels[x, y] = (r, min(255, int(g * 1.08)), b, a)
            elif r > 150 and g > 145 and b < 110:
                # Preserve extracted meter/bar yellow.
                pixels[x, y] = (min(255, int(r * 1.04)), min(255, int(g * 1.04)), b, a)
            elif mx < 95:
                pixels[x, y] = lavender
            else:
                # Keep odd anti-aliased edges readable without reintroducing paper.
                luminance = int(0.299 * r + 0.587 * g + 0.114 * b)
                if luminance < 130:
                    pixels[x, y] = lavender
                else:
                    pixels[x, y] = lavender_dim

    # Add subtle scanlines and a soft glow without changing geometry.
    overlay = Image.new("RGBA", image.size, (0, 0, 0, 0))
    opx = overlay.load()
    for y in range(0, height, 3):
        for x in range(width):
            opx[x, y] = (255, 255, 255, 14)
    image = Image.alpha_composite(image, overlay)

    out_path.parent.mkdir(parents=True, exist_ok=True)
    image.save(out_path)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--pdf", default="docs/m8-rgnano-parity/source/m8_operation_manual_v20260421.pdf")
    parser.add_argument("--out", default="docs/m8-rgnano-parity/assets/m8-manual")
    parser.add_argument("--scale", type=float, default=4.0)
    args = parser.parse_args()

    pdf_path = Path(args.pdf)
    out_dir = Path(args.out)
    ensure_pdf(pdf_path)

    doc = fitz.open(pdf_path)
    manifest: dict[str, Any] = {
        "manualUrl": MANUAL_URL,
        "pdf": str(pdf_path.as_posix()),
        "renderScale": args.scale,
        "screens": {},
    }

    for ref in SCREEN_REFS:
        page = doc[page_index(ref.manual_page)]
        candidates = screen_rect_candidates(page)
        rect = choose_rect(candidates, ref.rect_pick)
        image_name = f"m8-{ref.key}-manual-p{ref.manual_page:02d}.png"
        styled_name = f"m8-{ref.key}-rendered-p{ref.manual_page:02d}.png"
        image_path = out_dir / image_name
        styled_path = out_dir / styled_name
        image_size = render_crop(page, rect, args.scale, image_path)
        stylize_manual_crop(image_path, styled_path)
        text = extract_text_geometry(page, rect)
        drawings = extract_drawings(page, rect)

        manifest["screens"][ref.key] = {
            "label": ref.label,
            "manualPage": ref.manual_page,
            "endManualPage": ref.end_page,
            "pdfPage": pdf_page_number(ref.manual_page),
            "pdfPageIndex": page_index(ref.manual_page),
            "sourceUrl": f"{MANUAL_URL}#page={pdf_page_number(ref.manual_page)}",
            "image": f"assets/m8-manual/{image_name}",
            "styledImage": f"assets/m8-manual/{styled_name}",
            "imageSize": image_size,
            "cropRectPdfPoints": [round(v, 3) for v in (rect.x0, rect.y0, rect.x1, rect.y1)],
            "extraction": {
                "cropDetection": "largest stroked screen rectangle from PDF vector drawings",
                "pixelSource": "PyMuPDF render of the exact PDF crop rectangle",
                "textSource": "selectable PDF text inside crop, if present",
                "screenGlyphsAreVectorOutlines": len(text["chars"]) == 0,
                "characterOcrRequired": len(text["chars"]) == 0,
            },
            "candidateRectsPdfPoints": [
                [round(v, 3) for v in (candidate.x0, candidate.y0, candidate.x1, candidate.y1)]
                for candidate in candidates
            ],
            "text": text,
            "drawings": drawings,
        }

    manifest_path = out_dir / "m8-manual-screen-manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(f"Wrote {manifest_path}")
    print(f"Extracted {len(manifest['screens'])} screen crops")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
