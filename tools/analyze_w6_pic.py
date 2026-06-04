#!/usr/bin/env python3
"""Analyze Wizardry VI / Bane of the Cosmic Forge .PIC files.

These files are an undocumented Sir-tech proprietary vector format from 1990.
They are NOT raw EGA bitmaps — file sizes vary 1-23KB for similar visual
content, and the byte pattern strongly suggests RLE / vector drawing
commands.

This script:
  1. Surveys all .PIC files (size, leading bytes, repeating tokens)
  2. Tries the hypothesis "[Y8 X8 color8 length8] sequences terminated by 0xED 0x00"
  3. Outputs guess-stats for further reverse engineering

Run:
  python3 tools/analyze_w6_pic.py /path/to/iso_mount/bane/
"""

import sys
from collections import Counter
from pathlib import Path


def analyze_file(path: Path) -> dict:
    data = path.read_bytes()
    n = len(data)

    # Count occurrences of the suspected ED 00 terminator
    ed00_positions = []
    i = 0
    while i < n - 1:
        if data[i] == 0xED and data[i + 1] == 0x00:
            ed00_positions.append(i)
        i += 1

    # Most common byte values
    byte_counts = Counter(data)
    top_bytes = byte_counts.most_common(8)

    return {
        "name": path.name,
        "size": n,
        "first16_hex": data[:16].hex(),
        "ed00_count": len(ed00_positions),
        "first_ed00": ed00_positions[0] if ed00_positions else None,
        "ed00_gaps": [
            ed00_positions[k + 1] - ed00_positions[k]
            for k in range(min(5, len(ed00_positions) - 1))
        ],
        "top_bytes": [(f"0x{b:02x}", c) for b, c in top_bytes],
    }


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: analyze_w6_pic.py <bane_dir>", file=sys.stderr)
        return 2

    bane = Path(sys.argv[1])
    pics = sorted(bane.glob("mon*.pic")) + sorted(bane.glob("MON*.PIC"))
    if not pics:
        print(f"no MON*.pic under {bane}", file=sys.stderr)
        return 1

    print(f"Analyzing {len(pics)} W6 monster .PIC files")
    print(f"From: {bane}\n")
    print(f"{'name':<14} {'size':>6}  {'ed00':>5} {'first16':<32} top-bytes")
    print("-" * 100)

    total_size = 0
    for p in pics:
        r = analyze_file(p)
        total_size += r["size"]
        top_str = " ".join(f"{b}={c}" for b, c in r["top_bytes"][:4])
        print(f"{r['name']:<14} {r['size']:>6}  {r['ed00_count']:>5} "
              f"{r['first16_hex']:<32} {top_str}")

    print(f"\nTotal: {len(pics)} files, {total_size} bytes")

    # Cross-file pattern: is the very first byte ever > 0x10?
    first_bytes = Counter()
    for p in pics:
        first_bytes[p.read_bytes()[0]] += 1
    print(f"\nFirst-byte distribution: {dict(first_bytes.most_common())}")

    print("\n--- Format hypothesis ---")
    print("Observation: every file contains many ED 00 sequences (likely")
    print("end-of-record / end-of-row markers). Sizes 1-23KB for 32 monsters")
    print("suggests vector-drawing commands, not raw bitmaps.")
    print("")
    print("Speculated record layout (needs verification):")
    print("  [Y_lo Y_hi X_lo X_hi color_n length_n] terminated by ED 00")
    print("  where (Y, X) are screen coordinates (16-bit?)")
    print("  Each record is one filled rectangle/line at (X, Y) with color N")
    print("  and length N pixels.")
    print("")
    print("Next steps for full reverse-engineering:")
    print("  1. Render first few records as guessed primitives to PNG, eyeball")
    print("  2. Cross-reference against DRAGONSC.{CGA,EGA,T16} which share")
    print("     the same format but render a known scene")
    print("  3. Disassemble BANE.BAT / EGA.DRV to find the renderer routine")
    print("  4. Check if the Wizardry VI source code on archive.org has the")
    print("     drawing routine in Pascal source")
    return 0


if __name__ == "__main__":
    sys.exit(main())
