#!/usr/bin/env python3
"""
Walk docs/MONSTERS.md and insert each monster's PCE-CD sprite immediately
after its `### CHT (EN, ID N)` heading.

Uses raw.githubusercontent.com absolute URLs so the image renders in:
  - GitHub repo browser
  - Jekyll-served docs/ GitHub Pages site
  - any downstream Markdown viewer

Skips entries that already have a sprite block (idempotent — safe to re-run
after monsters.json gains new entries).
"""
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
MD = ROOT / "docs/MONSTERS.md"
DATA = ROOT / "assets/data/monsters.json"
REPO_RAW = "https://raw.githubusercontent.com/wicanr2/wizardry-1-cht/main"

def main():
    monsters = json.loads(DATA.read_text(encoding="utf-8"))
    id_to_sprite = {}
    id_to_name = {}
    for m in monsters:
        mid = m.get("id")
        sprite = m.get("sprite")
        if mid is None or not sprite:
            continue
        id_to_sprite[int(mid)] = sprite
        id_to_name[int(mid)] = m.get("name", "")

    text = MD.read_text(encoding="utf-8")
    out_lines = []
    inserted = 0
    skipped = 0
    # Allow trailing decoration after the closing paren (e.g. " ⚠️", " ★★★").
    heading_re = re.compile(r"^###\s+(.+?)\s+\(([^,]+),\s*ID\s+(\d+)\).*$")

    lines = text.split("\n")
    i = 0
    while i < len(lines):
        line = lines[i]
        out_lines.append(line)
        m = heading_re.match(line)
        if m:
            mid = int(m.group(3))
            en_name = m.group(2).strip()
            sprite_path = id_to_sprite.get(mid)
            # Look ahead one or two lines to see if a sprite block already exists.
            already = False
            for look in lines[i+1:i+4]:
                if "sprites/monsters_pcecd" in look or "PCECD_MS_" in look:
                    already = True
                    break
            if sprite_path and not already:
                # monsters.json stores sprite path relative to assets/,
                # so prepend "assets/" to get the repo-rooted URL.
                img_url = f"{REPO_RAW}/assets/{sprite_path}"
                out_lines.append("")
                out_lines.append(
                    f'<img src="{img_url}" '
                    f'width="120" alt="{en_name} PCE-CD sprite" '
                    f'align="right" />'
                )
                inserted += 1
            elif already:
                skipped += 1
        i += 1

    MD.write_text("\n".join(out_lines), encoding="utf-8")
    print(f"Inserted sprite for {inserted} monster headings; "
          f"skipped {skipped} that already had one.")

if __name__ == "__main__":
    main()
