#!/usr/bin/env python3
"""Render an Apple II HGR framebuffer (8KB at $2000-$3FFF) to a PNG.

The HGR mode is 280×192 pixels with NTSC colour-artifact "colors":
  - Each byte covers 7 pixels (bits 0..6); bit 7 is the high-bit / color shift
  - Even pixel + bit7=0 → violet (purple)
  - Even pixel + bit7=1 → blue
  - Odd  pixel + bit7=0 → green
  - Odd  pixel + bit7=1 → orange
  - Two adjacent on-pixels of the same set → white
  - Two adjacent off-pixels → black

Output: 280×192 PNG (or 560×384 if --scale 2 is given).
"""

import struct
import sys
import zlib
from pathlib import Path


def hgr_row_addr(row: int) -> int:
    return (
        ((row & 7) << 10)
        + ((row >> 3) & 7) * 0x80
        + ((row >> 6) & 3) * 0x28
    )


# Apple II NTSC artifact palette (BGR-ish picked for clarity)
PALETTE = {
    "black":  (0, 0, 0),
    "white":  (255, 255, 255),
    "violet": (160, 56, 224),
    "green":  (56, 224, 56),
    "blue":   (40, 96, 224),
    "orange": (224, 144, 40),
}


def render_rgb(hgr: bytes, color_mode: str = "ntsc") -> bytes:
    """Render 8192-byte HGR bytes → 280×192 RGB pixels."""
    width, height = 280, 192
    out = bytearray(width * height * 3)
    for row in range(height):
        base = hgr_row_addr(row)
        for col in range(40):  # 40 bytes per row
            byte = hgr[base + col]
            shift = (byte >> 7) & 1   # high-bit selects color set
            for bit in range(7):
                pixel_on = (byte >> bit) & 1
                # Pixel column (0..279)
                px = col * 7 + bit
                if color_mode == "mono":
                    rgb = PALETTE["white"] if pixel_on else PALETTE["black"]
                else:
                    if not pixel_on:
                        rgb = PALETTE["black"]
                    else:
                        is_even = (px & 1) == 0
                        if shift == 0:
                            rgb = PALETTE["violet"] if is_even else PALETTE["green"]
                        else:
                            rgb = PALETTE["blue"] if is_even else PALETTE["orange"]
                ofs = (row * width + px) * 3
                out[ofs : ofs + 3] = bytes(rgb)
    return bytes(out)


def write_png(rgb: bytes, width: int, height: int, path: Path, scale: int = 1):
    if scale > 1:
        # Nearest-neighbor upscale
        big = bytearray(width * scale * height * scale * 3)
        for r in range(height):
            for c in range(width):
                src = (r * width + c) * 3
                pix = rgb[src : src + 3]
                for dr in range(scale):
                    for dc in range(scale):
                        dst = ((r * scale + dr) * (width * scale) + (c * scale + dc)) * 3
                        big[dst : dst + 3] = pix
        rgb = bytes(big)
        width *= scale
        height *= scale

    def chunk(typ: bytes, data: bytes) -> bytes:
        crc = zlib.crc32(typ + data)
        return (
            struct.pack(">I", len(data))
            + typ
            + data
            + struct.pack(">I", crc)
        )

    sig = b"\x89PNG\r\n\x1a\n"
    ihdr = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    # Add 0x00 filter byte at start of each row
    raw = bytearray()
    for r in range(height):
        raw.append(0)
        raw.extend(rgb[r * width * 3 : (r + 1) * width * 3])
    idat = zlib.compress(bytes(raw), 9)

    path.write_bytes(sig + chunk(b"IHDR", ihdr) + chunk(b"IDAT", idat) + chunk(b"IEND", b""))


def main() -> int:
    if len(sys.argv) < 3:
        print("usage: hgr_to_png.py <input.hgr> <output.png> [--scale N] [--mono]", file=sys.stderr)
        return 2
    scale = 1
    color_mode = "ntsc"
    if "--scale" in sys.argv:
        i = sys.argv.index("--scale")
        scale = int(sys.argv[i + 1])
    if "--mono" in sys.argv:
        color_mode = "mono"

    hgr = Path(sys.argv[1]).read_bytes()
    if len(hgr) < 8192:
        hgr = hgr + bytes(8192 - len(hgr))
    elif len(hgr) > 8192:
        hgr = hgr[:8192]
    rgb = render_rgb(hgr, color_mode)
    write_png(rgb, 280, 192, Path(sys.argv[2]), scale=scale)
    print(f"wrote {sys.argv[2]}  (280×192{'×' + str(scale) if scale > 1 else ''})")
    return 0


if __name__ == "__main__":
    sys.exit(main())
