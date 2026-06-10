#!/usr/bin/env python3
"""Convert a CloneCD .ccd descriptor to a standard .cue sheet.

bchunk reads .cue / .bin, but the PCE-CD dumps we have are
.ccd / .img / .sub (CloneCD format). This emits an equivalent .cue
so we can feed bchunk without an extra tool.

Usage:
  tools/ccd_to_cue.py path/to/disc.ccd [path/to/disc.cue]
  (defaults to writing alongside the input)
"""

from __future__ import annotations

import re
import sys
from pathlib import Path


def parse_ccd(path: Path) -> list[dict]:
    text = path.read_text(encoding="latin-1")
    entries = re.findall(r"\[Entry \d+\](.*?)(?=\[|\Z)", text, re.S)
    tracks: list[dict] = []
    for e in entries:
        fields = dict(re.findall(r"(\w+)=([^\s]+)", e))
        raw = fields.get("Point", "0")
        pt = int(raw, 16) if raw.lower().startswith("0x") else int(raw)
        if 1 <= pt <= 0x63:
            tracks.append(
                {
                    "no": pt,
                    "ctl": int(fields.get("Control", "0x00"), 16),
                    "min": int(fields["PMin"]),
                    "sec": int(fields["PSec"]),
                    "frame": int(fields["PFrame"]),
                }
            )
    tracks.sort(key=lambda t: t["no"])
    return tracks


def write_cue(out: Path, img_name: str, tracks: list[dict]) -> None:
    with out.open("w") as f:
        f.write(f'FILE "{img_name}" BINARY\n')
        for t in tracks:
            mode = "MODE1/2352" if t["ctl"] & 0x04 else "AUDIO"
            f.write(f'  TRACK {t["no"]:02d} {mode}\n')
            f.write(
                f'    INDEX 01 {t["min"]:02d}:{t["sec"]:02d}:{t["frame"]:02d}\n'
            )


def main() -> int:
    if len(sys.argv) < 2:
        print(f"usage: {sys.argv[0]} <disc.ccd> [out.cue]", file=sys.stderr)
        return 1
    src = Path(sys.argv[1])
    if not src.exists():
        print(f"err: missing input: {src}", file=sys.stderr)
        return 1
    dst = Path(sys.argv[2]) if len(sys.argv) > 2 else src.with_suffix(".cue")
    img = src.with_suffix(".img").name
    tracks = parse_ccd(src)
    if not tracks:
        print("err: no tracks parsed from ccd", file=sys.stderr)
        return 1
    write_cue(dst, img, tracks)
    print(f"  ✓ wrote {dst} ({len(tracks)} tracks)")
    print(f"    next: bchunk -w \"{src.with_suffix('.img').name}\" \"{dst.name}\" out_")
    return 0


if __name__ == "__main__":
    sys.exit(main())
