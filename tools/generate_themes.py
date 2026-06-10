#!/usr/bin/env python3
"""Generate visual-theme variants of the PCE-CD monster sprites.

For each PNG in assets/sprites/monsters_pcecd/, write:
  assets/themes/mono/monsters/<name>.png    — 1-bit B&W (Apple II homage)
  assets/themes/outline/monsters/<name>.png — edge-detection line art
  assets/themes/sepia/monsters/<name>.png   — 4-colour CGA/DOS palette

These derivatives stay under the same CC-BY-SA 4.0 licence as the source
PCE-CD images (wizardry.wiki.gg, used by attribution + share-alike).
"""

from __future__ import annotations

import sys
from pathlib import Path

from PIL import Image, ImageChops, ImageFilter

ROOT     = Path(__file__).resolve().parent.parent
SRC_DIR  = ROOT / "assets" / "sprites" / "monsters_pcecd"
DST_BASE = ROOT / "assets" / "themes"

# CGA palette 1 (high intensity), the classic 4-colour DOS look.
SEPIA_PALETTE = [
    (0x00, 0x00, 0x00),
    (0x55, 0x55, 0x00),
    (0xAA, 0x55, 0x00),
    (0xFF, 0xFF, 0x55),
]


def split_alpha(im: Image.Image) -> tuple[Image.Image, Image.Image]:
    rgba = im.convert("RGBA")
    return rgba, rgba.split()[-1]


def make_mono(im: Image.Image) -> Image.Image:
    rgba, alpha = split_alpha(im)
    gray = rgba.convert("L")
    bw = gray.point(lambda p: 255 if p > 110 else 0).convert("RGB")
    return Image.merge("RGBA", (*bw.split(), alpha))


def make_outline(im: Image.Image) -> Image.Image:
    rgba, alpha = split_alpha(im)
    edges = rgba.convert("L").filter(ImageFilter.FIND_EDGES)
    edges = edges.point(lambda p: 255 if p > 32 else 0)
    out_alpha = ImageChops.multiply(edges, alpha)
    white = Image.new("RGB", im.size, (255, 255, 255))
    return Image.merge("RGBA", (*white.split(), out_alpha))


def make_sepia(im: Image.Image) -> Image.Image:
    rgba, alpha = split_alpha(im)
    pal_img = Image.new("P", (1, 1))
    flat: list[int] = []
    for c in SEPIA_PALETTE:
        flat.extend(c)
    flat.extend([0] * (768 - len(flat)))
    pal_img.putpalette(flat)
    q = rgba.convert("RGB").quantize(
        palette=pal_img, dither=Image.Dither.FLOYDSTEINBERG
    )
    rgb = q.convert("RGB")
    return Image.merge("RGBA", (*rgb.split(), alpha))


THEMES = {
    "mono":    make_mono,
    "outline": make_outline,
    "sepia":   make_sepia,
}


def process_one(src: Path) -> None:
    im = Image.open(src).convert("RGBA")
    for theme, fn in THEMES.items():
        out_dir = DST_BASE / theme / "monsters"
        out_dir.mkdir(parents=True, exist_ok=True)
        try:
            fn(im).save(out_dir / src.name, format="PNG")
        except Exception as e:  # noqa: BLE001
            print(f"  ✗ {theme}/{src.name}: {e}", file=sys.stderr)


def main() -> int:
    if not SRC_DIR.is_dir():
        print(f"err: source dir missing: {SRC_DIR}", file=sys.stderr)
        return 1
    pngs = sorted(SRC_DIR.glob("PCECD_MS_*.png"))
    if not pngs:
        print(f"err: no PCECD_MS_*.png in {SRC_DIR}", file=sys.stderr)
        return 1
    print(f"Generating derived themes from {len(pngs)} sprites...")
    for src in pngs:
        process_one(src)
    for theme in THEMES:
        print(f"  ✓ {theme:<8} -> {DST_BASE / theme / 'monsters'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
