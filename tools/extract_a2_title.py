#!/usr/bin/env python3
"""Extract Wizardry I 1981 title screen from Apple II disk image.

Pipeline:
  1. Read the .DSK image (143360-byte standard 35t/16s DOS 3.3 layout
     or UCSD Pascal P-System block format)
  2. Locate the LZ-compressed title screen blob (9 blocks = 4608 bytes
     per WTBUFF declaration in TITLELOA.TEXT)
  3. Decompress via lz_decomp.decode()
  4. Render HGR → PNG via hgr_to_png

This is an exploratory tool — the exact block offset of the title art in
WIZBOOT (Wiz1E.DSK) is not yet documented, so the script tries several
candidate offsets and saves the most "image-like" decode.

Usage:
  python3 extract_a2_title.py <DSK_file> <out_prefix>
"""

import sys
from pathlib import Path

# Make sibling tool importable
sys.path.insert(0, str(Path(__file__).parent))
from lz_decomp import decode  # noqa
from hgr_to_png import render_rgb, write_png  # noqa


def looks_like_image(hgr: bytes) -> float:
    """Heuristic score: how "structured" the HGR buffer looks.

    Real title art has non-zero bytes across many rows. Random/empty data
    has either all-zero or uniform-byte rows.
    """
    nonzero = sum(1 for b in hgr if b != 0 and b != 0xFF)
    unique = len(set(hgr))
    return nonzero / 8192.0 + unique / 256.0


def main() -> int:
    if len(sys.argv) < 3:
        print("usage: extract_a2_title.py <DSK> <out_prefix>", file=sys.stderr)
        return 2

    dsk = Path(sys.argv[1]).read_bytes()
    prefix = sys.argv[2]
    print(f"DSK size: {len(dsk)} bytes")

    # Candidate offsets: try every 256-byte block (rough scan)
    candidates = []
    for offset in range(0, len(dsk) - 4608, 256):
        # Quick check: byte at offset should be < 0xC0 (a row index) for a
        # valid LZ stream start.
        if dsk[offset] >= 0xC0:
            continue
        fb = decode(dsk, offset)
        hgr = fb.hgr_bytes()
        score = looks_like_image(hgr)
        if score > 0.1:
            candidates.append((score, offset, hgr))

    # Sort by score, keep top 5
    candidates.sort(reverse=True, key=lambda x: x[0])
    print(f"Found {len(candidates)} candidate offsets")

    for i, (score, offset, hgr) in enumerate(candidates[:5]):
        out_path = Path(f"{prefix}_offset{offset:06x}_score{score:.3f}.png")
        rgb = render_rgb(hgr)
        write_png(rgb, 280, 192, out_path, scale=2)
        print(f"  #{i+1}  offset=0x{offset:06x}  score={score:.3f} → {out_path}")

    if not candidates:
        print("No image-like decodes found.")
        print("This DSK may be:")
        print("  - UCSD Pascal P-System format (need P-System block table reader)")
        print("  - Compressed/encrypted differently")
        print("  - Title art is stored as raw HGR (not LZ)")

    return 0


if __name__ == "__main__":
    sys.exit(main())
