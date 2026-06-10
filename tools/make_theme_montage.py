#!/usr/bin/env python3
"""Render docs/themes_montage.png — a comparison grid of every theme.

Rows   = themes (PCE-CD, Mono, Outline, Sepia)
Cols   = a curated set of representative monsters
Output = docs/themes_montage.png (committable; derived from CC-BY-SA source)
"""

from __future__ import annotations

import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT   = Path(__file__).resolve().parent.parent
ASSETS = ROOT / "assets"
OUT    = ROOT / "docs" / "themes_montage.png"

# Curated lineup — one entry per "tier" of the dungeon for visual variety.
MONSTERS = [
    ("Bubbly_Slime",   "氣泡黏液"),
    ("Kobold",         "狗頭人"),
    ("Orc",            "獸人"),
    ("Lvl1_Mage",      "見習魔法師"),
    ("Greater_Demon",  "大惡魔"),
    ("Vampire_Lord",   "吸血鬼領主"),
    ("Werdna",         "瘋王 Werdna"),
]

# (theme_id, display label, dir under assets/)
ROWS = [
    ("pcecd",   "PCE-CD (彩色原版)",  "sprites/monsters_pcecd"),
    ("mono",    "Mono (黑白致敬)",     "themes/mono/monsters"),
    ("outline", "Outline (線稿致敬)",  "themes/outline/monsters"),
    ("sepia",   "Sepia (CGA / DOS)",  "themes/sepia/monsters"),
]

CELL_W, CELL_H = 140, 140
LABEL_W = 220
HEADER_H = 36
PADDING = 12
BG = (28, 28, 36)
FG = (235, 235, 240)
ACCENT = (210, 180, 80)


def find_font(size: int) -> ImageFont.ImageFont:
    candidates = [
        ASSETS / "fonts" / "NotoSansCJK-Regular.ttc",
        Path("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"),
        Path("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"),
    ]
    for p in candidates:
        if p.exists():
            try:
                return ImageFont.truetype(str(p), size)
            except Exception:  # noqa: BLE001
                continue
    return ImageFont.load_default()


def main() -> int:
    title_font = find_font(22)
    label_font = find_font(15)
    name_font  = find_font(13)

    w = LABEL_W + CELL_W * len(MONSTERS) + PADDING * 2
    h = HEADER_H + CELL_H * len(ROWS) + PADDING * 2 + HEADER_H
    canvas = Image.new("RGB", (w, h), BG)
    draw = ImageDraw.Draw(canvas)

    draw.text(
        (PADDING, PADDING - 2),
        "巫術 I — F3 主題切換對照表 (Theme Comparison)",
        font=title_font, fill=ACCENT,
    )

    # Monster name headers across the top.
    y_header = HEADER_H + PADDING
    for ci, (_mid, mname) in enumerate(MONSTERS):
        x = LABEL_W + PADDING + ci * CELL_W + CELL_W // 2
        bbox = draw.textbbox((0, 0), mname, font=name_font)
        tw = bbox[2] - bbox[0]
        draw.text((x - tw // 2, y_header - 18), mname, font=name_font, fill=FG)

    missing = 0
    for ri, (_tid, tlabel, tdir) in enumerate(ROWS):
        row_y = y_header + ri * CELL_H
        draw.text((PADDING, row_y + CELL_H // 2 - 8),
                  tlabel, font=label_font, fill=FG)
        base = ASSETS / tdir
        for ci, (mid, _mname) in enumerate(MONSTERS):
            cell_x = LABEL_W + PADDING + ci * CELL_W
            png = base / f"PCECD_MS_{mid}.png"
            if not png.exists():
                missing += 1
                draw.rectangle(
                    [cell_x + 8, row_y + 8,
                     cell_x + CELL_W - 8, row_y + CELL_H - 8],
                    outline=(80, 80, 90),
                )
                draw.text(
                    (cell_x + CELL_W // 2 - 14, row_y + CELL_H // 2 - 8),
                    "—", font=label_font, fill=(120, 120, 130),
                )
                continue
            im = Image.open(png).convert("RGBA")
            # Fit into cell preserving aspect.
            iw, ih = im.size
            scale = min((CELL_W - 16) / iw, (CELL_H - 16) / ih)
            tw, th = int(iw * scale), int(ih * scale)
            thumb = im.resize((tw, th), Image.LANCZOS)
            paste_x = cell_x + (CELL_W - tw) // 2
            paste_y = row_y + (CELL_H - th) // 2
            canvas.paste(thumb, (paste_x, paste_y), thumb)

    # Footer credit.
    footer = (
        "Source: PCE-CD by Sueyoshi Jun / Naxat Soft (1993) "
        "via wizardry.wiki.gg (CC-BY-SA 4.0).  "
        "Mono / Outline / Sepia 為衍生作品，授權繼承。"
    )
    draw.text(
        (PADDING, h - HEADER_H + 6),
        footer, font=name_font, fill=(170, 170, 180),
    )

    OUT.parent.mkdir(parents=True, exist_ok=True)
    canvas.save(OUT, format="PNG", optimize=True)
    print(f"  ✓ wrote {OUT.relative_to(ROOT)}  ({OUT.stat().st_size} bytes)")
    if missing:
        print(f"  ! {missing} cells empty — run tools/generate_themes.py first")
    return 0


if __name__ == "__main__":
    sys.exit(main())
