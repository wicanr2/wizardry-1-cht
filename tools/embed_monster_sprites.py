#!/usr/bin/env python3
"""
Walk docs/MONSTERS.md and insert (or refresh) the pre-composed 4-up sprite
PNG right after each `### CHT (EN, ID N)` heading. The 4-up PNG shows the
four ship-ready visual themes side-by-side:

    PCE-CD master | Mono | Outline | Sepia

so the reader sees what F3 actually swaps. The 4-up PNGs are produced by
tools/build_4up_sprites.py and live under docs/sprites_4up/ (relative
path so it works both in repo browser AND Jekyll Pages — the derivative
theme dirs themselves are gitignored, hence the pre-compose approach).

Idempotent: re-running replaces the previous embed block (single img,
4-up <table>, or this 4-up <img>) with the latest. Safe to run again
after monsters.json grows new entries.
"""
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
MD = ROOT / "docs/MONSTERS.md"
DATA = ROOT / "assets/data/monsters.json"

def fourup_block(sprite_path: str, en_name: str) -> list[str]:
    """sprite_path comes from monsters.json (e.g.
    'sprites/monsters_pcecd/PCECD_MS_X.png'). The composed 4-up PNG
    under docs/sprites_4up/ uses the same basename."""
    basename = Path(sprite_path).name  # PCECD_MS_X.png
    rel_url = f"sprites_4up/{basename}"
    return [
        f'<img src="{rel_url}" width="384" '
        f'alt="{en_name} — PCE-CD / Mono / Outline / Sepia" '
        f'align="right" />',
    ]

# Detect every flavour of existing embed:
#   - <table align="right"> ... </table>  (the previous 4-up table style)
#   - <img ... PCE-CD sprite" align="right" />  (the first single-img pass)
#   - <img ... monsters_pcecd ... align="right" />  (any older single-img)
#   - <img ... sprites_4up/... align="right" />  (the current 4-up PNG)
def is_existing_embed_start(line: str) -> bool:
    return (line.startswith('<table align="right"')
            or ('<img src=' in line
                and ('PCE-CD' in line
                     or 'monsters_pcecd' in line
                     or 'sprites_4up' in line)))

def collect_embed_block(lines, start):
    """Returns (start_idx, end_idx_exclusive) covering one prior embed block
    plus the trailing blank line if present. Caller deletes [start:end)."""
    i = start
    if lines[i].startswith('<table align="right"'):
        # Multi-line table — scan to </table>.
        while i < len(lines) and '</table>' not in lines[i]:
            i += 1
        i += 1  # include the </table> line
    else:
        # Single <img> line.
        i += 1
    # Eat one trailing blank line so we don't leave double blanks.
    if i < len(lines) and lines[i].strip() == "":
        i += 1
    return start, i

def main():
    monsters = json.loads(DATA.read_text(encoding="utf-8"))
    id_to_sprite = {}
    for m in monsters:
        mid = m.get("id")
        sprite = m.get("sprite")
        if mid is None or not sprite:
            continue
        id_to_sprite[int(mid)] = sprite

    text = MD.read_text(encoding="utf-8")
    lines = text.split("\n")
    heading_re = re.compile(r"^###\s+(.+?)\s+\(([^,]+),\s*ID\s+(\d+)\).*$")

    out = []
    i = 0
    inserted = 0
    replaced = 0
    while i < len(lines):
        line = lines[i]
        m = heading_re.match(line)
        if not m:
            out.append(line)
            i += 1
            continue
        # Emit heading + blank line.
        out.append(line)
        i += 1
        if i < len(lines) and lines[i].strip() == "":
            out.append(lines[i])
            i += 1

        mid = int(m.group(3))
        en_name = m.group(2).strip()
        sprite_path = id_to_sprite.get(mid)
        if not sprite_path:
            continue

        # Skip over any existing embed block.
        had_old = False
        while i < len(lines) and is_existing_embed_start(lines[i]):
            had_old = True
            _, end = collect_embed_block(lines, i)
            i = end

        # Insert fresh 4-up block.
        for b in fourup_block(sprite_path, en_name):
            out.append(b)
        out.append("")  # blank line after the block

        if had_old:
            replaced += 1
        else:
            inserted += 1

    MD.write_text("\n".join(out), encoding="utf-8")
    print(f"4-up sprite strip: {inserted} new, {replaced} refreshed.")

if __name__ == "__main__":
    main()
