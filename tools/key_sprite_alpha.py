#!/usr/bin/env python3
"""
Key out the solid-black background on PCE-CD monster sprites so they render
transparently over the 3D maze view in combat.

The PCE-CD master PNGs (assets/sprites/monsters_pcecd/*.png) ship as RGBA
but with `(0,0,0,255)` corners — the original PC-Engine CD-ROM master had
the monster on a black background and that background was never alpha-keyed
when the PNGs were extracted. Combat scene draws maze view as the backdrop
and overlays the sprite on top; the solid black band covered the corridor.

This tool does an in-place RGBA alpha-key pass: any pixel within
`tolerance` Euclidean distance of `key_rgb` gets its alpha zeroed.

Default key = (0, 0, 0) and tolerance = 0 (exact match only). PCE-CD sprite
bodies are saturated colours, never pure (0,0,0) outside the background, so
this is safe — monster details survive untouched.

Usage:
  python3 tools/key_sprite_alpha.py
  python3 tools/key_sprite_alpha.py --dir assets/themes/sepia/monsters
  python3 tools/key_sprite_alpha.py --tol 6        # allow near-black too
"""
import argparse
import sys
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parent.parent
DEFAULT_DIR = ROOT / "assets/sprites/monsters_pcecd"

def key_one(png_path: Path, key_rgb=(0, 0, 0), tol: int = 0) -> int:
    """Return number of pixels keyed to transparent."""
    im = Image.open(png_path).convert("RGBA")
    px = im.load()
    n = 0
    tol2 = tol * tol
    kr, kg, kb = key_rgb
    for y in range(im.height):
        for x in range(im.width):
            r, g, b, a = px[x, y]
            if a == 0:
                continue
            dr, dg, db = r - kr, g - kg, b - kb
            d2 = dr*dr + dg*dg + db*db
            if d2 <= tol2:
                px[x, y] = (r, g, b, 0)
                n += 1
    im.save(png_path, "PNG", optimize=True)
    return n

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dir", default=str(DEFAULT_DIR))
    ap.add_argument("--key", default="0,0,0",
                    help="comma-separated RGB to treat as background")
    ap.add_argument("--tol", type=int, default=0,
                    help="Euclidean tolerance — 0 = exact match only")
    args = ap.parse_args()
    d = Path(args.dir)
    if not d.is_dir():
        print(f"error: not a directory: {d}", file=sys.stderr)
        sys.exit(1)
    key = tuple(int(s) for s in args.key.split(","))
    if len(key) != 3:
        print(f"error: --key must be 'r,g,b'", file=sys.stderr)
        sys.exit(1)

    total_files = 0
    total_pixels = 0
    for p in sorted(d.glob("*.png")):
        n = key_one(p, key_rgb=key, tol=args.tol)
        if n > 0:
            print(f"  {p.name}: {n} px keyed")
        total_files += 1
        total_pixels += n
    print(f"\nProcessed {total_files} PNGs in {d}, keyed {total_pixels:,} pixels.")

if __name__ == "__main__":
    main()
