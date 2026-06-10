#!/usr/bin/env python3
"""Render docs/maps/floor_B[1-10]F.svg from assets/data/wiz1_mazes.json.

One SVG per floor, 20×20 grid, walls/doors as line segments and
special squares colour-coded with a Chinese-labelled legend.
"""

from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "assets" / "data" / "wiz1_mazes.json"
OUT_DIR = ROOT / "docs" / "maps"

SIZE = 20
CELL = 28  # px per cell
MARGIN = 32

# Feature → (background colour, glyph, Chinese label)
FEATURE_STYLE = {
    "stairs_up":    ("#4CAF50", "↑", "上樓"),
    "stairs_down":  ("#2196F3", "↓", "下樓"),
    "elevator":     ("#9C27B0", "E", "電梯"),
    "pit":          ("#795548", "P", "陷阱坑"),
    "chute":        ("#FF9800", "S", "滑梯"),
    "spinner":      ("#FFC107", "T", "旋轉盤"),
    "teleporter":   ("#E91E63", "M", "傳送門"),
    "key_item":     ("#FFD700", "K", "關鍵道具"),
    "marker":       ("#607D8B", "•", "標記點"),
}


def cell_rect(x: int, y: int) -> tuple[int, int]:
    # Flip y so y=0 sits at the bottom of the canvas, matching the PDF.
    px = MARGIN + x * CELL
    py = MARGIN + (SIZE - 1 - y) * CELL
    return px, py


def wall_segments(cells: dict[tuple[int, int], dict]) -> list[str]:
    """Emit one SVG <line> per real wall (any of n/e/s/w != 'open' or default outer rim)."""
    segs: list[str] = []
    for y in range(SIZE):
        for x in range(SIZE):
            c = cells.get((x, y), {})
            n = c.get("n", "open")
            e = c.get("e", "open")
            s = c.get("s", "open")
            w = c.get("w", "open")
            # Outer-rim defaults (only when no explicit value given).
            if x == 0 and "w" not in c: w = "wall"
            if x == SIZE - 1 and "e" not in c: e = "wall"
            if y == 0 and "s" not in c: s = "wall"
            if y == SIZE - 1 and "n" not in c: n = "wall"

            px, py = cell_rect(x, y)
            x0, y0, x1, y1 = px, py, px + CELL, py + CELL
            # n=top edge, s=bottom edge of *this* cell (after y-flip)
            if n != "open":
                stroke = "#000" if n == "wall" else "#888"
                dash = "" if n == "wall" else 'stroke-dasharray="4,3"'
                segs.append(f'<line x1="{x0}" y1="{y0}" x2="{x1}" y2="{y0}" stroke="{stroke}" stroke-width="2" {dash}/>')
            if s != "open":
                stroke = "#000" if s == "wall" else "#888"
                dash = "" if s == "wall" else 'stroke-dasharray="4,3"'
                segs.append(f'<line x1="{x0}" y1="{y1}" x2="{x1}" y2="{y1}" stroke="{stroke}" stroke-width="2" {dash}/>')
            if w != "open":
                stroke = "#000" if w == "wall" else "#888"
                dash = "" if w == "wall" else 'stroke-dasharray="4,3"'
                segs.append(f'<line x1="{x0}" y1="{y0}" x2="{x0}" y2="{y1}" stroke="{stroke}" stroke-width="2" {dash}/>')
            if e != "open":
                stroke = "#000" if e == "wall" else "#888"
                dash = "" if e == "wall" else 'stroke-dasharray="4,3"'
                segs.append(f'<line x1="{x1}" y1="{y0}" x2="{x1}" y2="{y1}" stroke="{stroke}" stroke-width="2" {dash}/>')
    return segs


def feature_marks(cells: dict[tuple[int, int], dict]) -> list[str]:
    marks: list[str] = []
    for (x, y), c in cells.items():
        if c.get("dark"):
            px, py = cell_rect(x, y)
            marks.append(f'<rect x="{px}" y="{py}" width="{CELL}" height="{CELL}" '
                         f'fill="#ddd" fill-opacity="0.55"/>')
        feat = c.get("feature")
        if not feat:
            continue
        style = FEATURE_STYLE.get(feat)
        if not style:
            continue
        bg, glyph, _label = style
        px, py = cell_rect(x, y)
        cx, cy = px + CELL // 2, py + CELL // 2
        marks.append(
            f'<circle cx="{cx}" cy="{cy}" r="{CELL // 2 - 4}" fill="{bg}" stroke="#222" stroke-width="1"/>'
            f'<text x="{cx}" y="{cy + 5}" font-size="14" font-weight="bold" '
            f'fill="white" text-anchor="middle">{glyph}</text>'
        )
        label = c.get("label")
        if label:
            marks.append(
                f'<text x="{px + 4}" y="{py + 11}" font-size="9" fill="#222">{label}</text>'
            )
    return marks


def axis_labels() -> list[str]:
    lines: list[str] = []
    for i in range(SIZE):
        # bottom: x labels
        px = MARGIN + i * CELL + CELL // 2
        py = MARGIN + SIZE * CELL + 14
        lines.append(
            f'<text x="{px}" y="{py}" font-size="10" fill="#555" text-anchor="middle">{i}</text>'
        )
        # left: y labels
        px = MARGIN - 12
        py = MARGIN + (SIZE - 1 - i) * CELL + CELL // 2 + 4
        lines.append(
            f'<text x="{px}" y="{py}" font-size="10" fill="#555" text-anchor="middle">{i}</text>'
        )
    return lines


def legend(canvas_w: int) -> list[str]:
    lines: list[str] = []
    x_start = canvas_w + 20
    y = MARGIN
    lines.append(f'<text x="{x_start}" y="{y}" font-size="13" font-weight="bold" fill="#222">圖例</text>')
    y += 16
    for _key, (bg, glyph, label) in FEATURE_STYLE.items():
        lines.append(
            f'<circle cx="{x_start + 10}" cy="{y - 4}" r="9" fill="{bg}" stroke="#222" stroke-width="1"/>'
            f'<text x="{x_start + 10}" y="{y}" font-size="11" font-weight="bold" fill="white" text-anchor="middle">{glyph}</text>'
            f'<text x="{x_start + 28}" y="{y + 1}" font-size="11" fill="#222">{label}</text>'
        )
        y += 20
    return lines


def render_floor(floor: dict) -> str:
    cells_list = floor.get("cells", [])
    cells = {(c["x"], c["y"]): c for c in cells_list}

    canvas_w = MARGIN * 2 + SIZE * CELL
    canvas_h = MARGIN * 2 + SIZE * CELL + 24

    parts = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{canvas_w + 140}" height="{canvas_h}" font-family="sans-serif">',
        f'<rect width="100%" height="100%" fill="#fafafa"/>',
        f'<text x="{MARGIN}" y="{MARGIN - 10}" font-size="14" font-weight="bold" fill="#222">'
        f'巫術 I — {floor.get("name", "")} (B{floor["level"]}F)</text>',
    ]
    parts += axis_labels()
    parts += feature_marks(cells)
    parts += wall_segments(cells)
    parts += legend(canvas_w)
    parts.append('</svg>')
    return "\n".join(parts)


def main() -> int:
    data = json.loads(SRC.read_text())
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    for floor in data["floors"]:
        svg = render_floor(floor)
        out = OUT_DIR / f"floor_B{floor['level']:02d}F.svg"
        out.write_text(svg)
        print(f"  ✓ wrote {out.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
