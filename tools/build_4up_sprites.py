#!/usr/bin/env python3
"""
For each monster in monsters.json, compose a single 4-up PNG showing the
four ship-ready visual themes side-by-side:

    PCE-CD master | Mono | Outline | Sepia

Output: docs/sprites_4up/<basename>.png

Why one combined file instead of four inline <img>s pointing at the theme
dirs: assets/themes/{mono,outline,sepia}/ are gitignored (derivatives that
regenerate locally), so absolute-URL embeds 404 on GitHub Pages. Bundling
one composed PNG per monster under docs/ keeps the repo light (~120 KB
total for 30 monsters) AND keeps the docs URLs self-contained.

Each cell is 96×112 px:
- Top 96×96: the sprite, "fit" into the box, anchored bottom-centre so the
  monster's feet line up across themes (sprite native size 64×96 → fits
  with horizontal padding).
- Bottom 16 px: text label (PCE-CD / Mono / Outline / Sepia) in 11 px font.

Run from repo root:  python3 tools/build_4up_sprites.py
"""
import json
import sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
DATA = ROOT / "assets/data/monsters.json"
OUT_DIR = ROOT / "docs/sprites_4up"

THEMES = [
    ("sprites/monsters_pcecd",   "PCE-CD"),
    ("themes/mono/monsters",     "Mono"),
    ("themes/outline/monsters",  "Outline"),
    ("themes/sepia/monsters",    "Sepia"),
]

CELL_W, CELL_H = 96, 112
SPRITE_BOX_H = 96
LABEL_H = CELL_H - SPRITE_BOX_H  # 16
BG = (18, 18, 26, 255)  # dark slate so transparent sprites read

def load_font(size=12):
    """Pillow Hershey fallback if no TTF is on the system."""
    for candidate in [
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
    ]:
        if Path(candidate).exists():
            return ImageFont.truetype(candidate, size)
    return ImageFont.load_default()

def fit_sprite_into_cell(src_path: Path, cell: Image.Image, cell_x: int):
    if not src_path.exists():
        # Placeholder X.
        d = ImageDraw.Draw(cell)
        d.rectangle([cell_x + 4, 4, cell_x + CELL_W - 4, SPRITE_BOX_H - 4],
                    outline=(80, 80, 90, 255))
        d.line([(cell_x + 4, 4), (cell_x + CELL_W - 4, SPRITE_BOX_H - 4)],
               fill=(120, 60, 60, 255), width=2)
        d.line([(cell_x + CELL_W - 4, 4), (cell_x + 4, SPRITE_BOX_H - 4)],
               fill=(120, 60, 60, 255), width=2)
        return
    sprite = Image.open(src_path).convert("RGBA")
    # Fit by scale.
    sw, sh = sprite.size
    scale = min((CELL_W - 8) / sw, (SPRITE_BOX_H - 8) / sh, 1.0)
    new = (max(1, int(sw * scale)), max(1, int(sh * scale)))
    if new != sprite.size:
        sprite = sprite.resize(new, Image.LANCZOS)
    # Anchor bottom-centre.
    dx = cell_x + (CELL_W - sprite.width) // 2
    dy = SPRITE_BOX_H - sprite.height - 2
    cell.alpha_composite(sprite, (dx, dy))

def compose_one(sprite_rel: str, out_path: Path, font: ImageFont.ImageFont):
    """sprite_rel is e.g. 'sprites/monsters_pcecd/PCECD_MS_X.png' — same
    basename used across all theme dirs."""
    basename = Path(sprite_rel).name
    canvas = Image.new("RGBA", (CELL_W * len(THEMES), CELL_H), BG)
    for idx, (theme_dir, _label) in enumerate(THEMES):
        src = ROOT / "assets" / theme_dir / basename
        fit_sprite_into_cell(src, canvas, idx * CELL_W)
    # Label strip — semi-translucent black band + white text.
    d = ImageDraw.Draw(canvas)
    d.rectangle([0, SPRITE_BOX_H, canvas.width, CELL_H],
                fill=(0, 0, 0, 200))
    for idx, (_theme_dir, label) in enumerate(THEMES):
        # Centre label in its column.
        bbox = d.textbbox((0, 0), label, font=font)
        tw = bbox[2] - bbox[0]
        tx = idx * CELL_W + (CELL_W - tw) // 2
        d.text((tx, SPRITE_BOX_H + 1), label,
               fill=(230, 230, 235, 255), font=font)
    canvas.convert("RGB").save(out_path, "PNG", optimize=True)

def main():
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    monsters = json.loads(DATA.read_text(encoding="utf-8"))
    font = load_font(11)
    wrote = 0
    for m in monsters:
        sprite = m.get("sprite")
        if not sprite:
            continue
        basename = Path(sprite).name
        out_path = OUT_DIR / basename  # keep same filename
        compose_one(sprite, out_path, font)
        wrote += 1
    print(f"Wrote {wrote} 4-up sprite PNGs to {OUT_DIR}.")

if __name__ == "__main__":
    main()
