#!/usr/bin/env python3
"""Compose a stylised title-screen background from existing PCE-CD assets.

Placeholder for the actual PCE-CD title-screen ripped from the data
track once a tool exists to decode HuC6280 graphics format. For now we
build a 1280×720 PNG out of:
  - dark dungeon-stone gradient
  - centred Werdna silhouette (the antagonist) from the bundled sprites
  - vignetted frame
  - Chinese + English subtitle slot near the bottom

Output: assets/themes/pcecd/title/background.png (committed; CC-BY-SA
derivative of the PCE-CD source set, same as the monster sprites).
"""

from __future__ import annotations

import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = Path(__file__).resolve().parent.parent
SRC_SPRITE = ROOT / "assets" / "sprites" / "monsters_pcecd" / "PCECD_MS_Werdna.png"
OUT = ROOT / "assets" / "themes" / "pcecd" / "title" / "background.png"

W, H = 1280, 720


def find_font(size: int) -> ImageFont.ImageFont:
    candidates = [
        ROOT / "assets" / "fonts" / "NotoSansCJK-Regular.ttc",
        Path("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"),
    ]
    for p in candidates:
        if p.exists():
            try:
                return ImageFont.truetype(str(p), size)
            except Exception:  # noqa: BLE001
                continue
    return ImageFont.load_default()


def stone_gradient(w: int, h: int) -> Image.Image:
    base = Image.new("RGB", (w, h), (0, 0, 0))
    px = base.load()
    for y in range(h):
        t = y / max(1, h - 1)
        # Top = deep navy, bottom = warm dark brown for "torchlight on stone".
        r = int(8  + (40 - 8)  * t)
        g = int(10 + (24 - 10) * t)
        b = int(20 + (20 - 20) * t)
        for x in range(w):
            # Add radial vignette darkening at the edges
            dx = (x - w / 2) / (w / 2)
            dy = (y - h / 2) / (h / 2)
            vig = 1.0 - 0.35 * min(1.0, dx * dx + dy * dy)
            px[x, y] = (int(r * vig), int(g * vig), int(b * vig))
    return base


def main() -> int:
    if not SRC_SPRITE.exists():
        print(f"err: missing source sprite {SRC_SPRITE}", file=sys.stderr)
        return 1

    canvas = stone_gradient(W, H)
    draw = ImageDraw.Draw(canvas)

    # Werdna silhouette, scaled and centred.
    werdna = Image.open(SRC_SPRITE).convert("RGBA")
    wr_h = 520
    wr_w = int(werdna.width * wr_h / werdna.height)
    werdna = werdna.resize((wr_w, wr_h), Image.LANCZOS)
    # Slight darken pass to fit the dungeon mood.
    darkened = Image.eval(werdna.convert("RGB"), lambda v: int(v * 0.65))
    werdna = Image.merge("RGBA", (*darkened.split(), werdna.split()[-1]))
    paste_x = (W - wr_w) // 2
    paste_y = (H - wr_h) // 2 - 30
    canvas.paste(werdna, (paste_x, paste_y), werdna)

    # Outer frame.
    border = (140, 110, 70)
    for i in range(4):
        draw.rectangle(
            [16 + i, 16 + i, W - 17 - i, H - 17 - i],
            outline=border, width=1,
        )

    # Title text.
    title_font_zh = find_font(64)
    title_font_en = find_font(36)
    sub_font = find_font(20)

    title_zh = "巫術 — 瘋王的試煉場"
    title_en = "WIZARDRY: Proving Grounds of the Mad Overlord"
    subtitle = "Sir-Tech 1981  /  C++17 SDL2 繁中重寫  /  PCE-CD theme"

    # Centre the Chinese title near the top.
    bbox = draw.textbbox((0, 0), title_zh, font=title_font_zh)
    tw = bbox[2] - bbox[0]
    draw.text(
        ((W - tw) // 2, 80),
        title_zh, font=title_font_zh, fill=(220, 180, 90),
        stroke_width=2, stroke_fill=(0, 0, 0),
    )

    # English subtitle below.
    bbox = draw.textbbox((0, 0), title_en, font=title_font_en)
    tw = bbox[2] - bbox[0]
    draw.text(
        ((W - tw) // 2, 170),
        title_en, font=title_font_en, fill=(190, 160, 110),
        stroke_width=1, stroke_fill=(0, 0, 0),
    )

    # Footer.
    bbox = draw.textbbox((0, 0), subtitle, font=sub_font)
    tw = bbox[2] - bbox[0]
    draw.text(
        ((W - tw) // 2, H - 60),
        subtitle, font=sub_font, fill=(170, 150, 120),
    )

    OUT.parent.mkdir(parents=True, exist_ok=True)
    canvas.save(OUT, format="PNG", optimize=True)
    print(f"  ✓ wrote {OUT.relative_to(ROOT)}  ({OUT.stat().st_size} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
