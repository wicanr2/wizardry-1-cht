#!/usr/bin/env python3
"""Python port of LZDECOMP.TEXT (Apple II 6502 assembly) from snafaru/Wizardry.Code.

Source: /home/anr2/wizardy-cht/Wizardry.Code/Wiz1E.DSK/LZDECOMP.TEXT.txt

The algorithm decompresses Wizardry I title screen art directly into the
Apple II hi-res framebuffer (8KB at $2000-$3FFF). This Python port produces
the equivalent framebuffer as a bytes object that `hgr_to_png.py` can render.

Algorithm summary:
  Tokens at current input pointer:
    - byte < 0xC0  → row index; read 1-byte bitmask + up to 5 literal bytes
    - 0xC0..0xFC   → (col_offset, hi-byte-row, value) tuple → write one byte
    - 0xFD         → XOR-toggle high bit on 0x30+1 rows (visual flash effect)
    - 0xFE         → scroll-left 0x30+1 rows (animation)
    - 0xFF or other → end of stream
"""

import sys
from pathlib import Path


# Apple II HGR row → base address tables.
# Apple HGR is non-linear: 192 rows interleaved across $2000-$3FFF.
# Formula: addr = $2000 + ((row & 7) << 10) + ((row >> 3) & 7) * 0x80
#                 + ((row >> 6) & 3) * 0x28
def hgr_row_addr(row: int) -> int:
    return (
        0x2000
        + ((row & 7) << 10)
        + ((row >> 3) & 7) * 0x80
        + ((row >> 6) & 3) * 0x28
    )


def build_hgr_tables():
    """Build L04BC (low byte) and L057C (high byte) tables for rows 0..191."""
    lo = [0] * 192
    hi = [0] * 192
    for r in range(192):
        a = hgr_row_addr(r)
        lo[r] = a & 0xFF
        hi[r] = (a >> 8) & 0xFF
    return lo, hi


HGR_LO, HGR_HI = build_hgr_tables()


class Framebuffer:
    """Sparse 8KB framebuffer addressed by absolute $2000..$3FFF address."""

    def __init__(self):
        self.mem = bytearray(0x4000)  # cover $0000-$3FFF for simplicity

    def write(self, addr: int, val: int):
        if 0 <= addr < len(self.mem):
            self.mem[addr] = val & 0xFF

    def read(self, addr: int) -> int:
        return self.mem[addr] if 0 <= addr < len(self.mem) else 0

    def hgr_bytes(self) -> bytes:
        """Return $2000-$3FFF as 8192-byte HGR page."""
        return bytes(self.mem[0x2000:0x4000])


def decode(data: bytes, start_offset: int = 0) -> Framebuffer:
    """Decode an LZ blob starting at `data[start_offset]`.

    Returns the populated framebuffer.
    """
    fb = Framebuffer()
    p = start_offset
    n = len(data)

    while p < n:
        b = data[p]

        if b < 0xC0:
            # Token type 1: literal row
            # b = row index (0..0xBF, but only 0..191 are valid HGR rows)
            if b >= 192:
                # Out of HGR range — likely end or corrupt
                break
            row = b
            base = (HGR_HI[row] << 8) | HGR_LO[row]
            if p + 6 >= n:
                break
            mask = data[p + 1]
            # 8 bits in mask correspond to 8 byte slots; if bit set, take from input
            cur = p + 2
            for bit in range(8):
                if mask & 1:
                    if cur < n:
                        fb.write(base + bit, data[cur])
                        cur += 1
                mask >>= 1
            p = cur

        elif b < 0xFD:
            # Token type 2: col offset + (row_hi << 8 | val) ... single-byte write
            # Source: SEC; SBC #C0; STA 07 (col offset)
            #         LDY #02; LDA @02,Y → 06 (value byte)
            #         DEY; LDA @02,Y → row index
            #         LDA HGR_LO[row], HGR_HI[row] → base
            #         STA base[col_offset]
            col_off = b - 0xC0
            if p + 2 >= n:
                break
            value = data[p + 2]
            row = data[p + 1]
            if row < 192:
                base = (HGR_HI[row] << 8) | HGR_LO[row]
                fb.write(base + col_off, value)
            p += 3

        elif b == 0xFE:
            # Scroll-left effect (animation frame); skip for static image
            # The original asm does 0x30+1 = 49 scrolls. For static PNG we just
            # advance past the marker.
            p += 1

        elif b == 0xFD:
            # XOR-flash effect; same — skip
            p += 1

        else:
            # 0xFF or unknown: end of stream
            break

    return fb


def main() -> int:
    if len(sys.argv) < 3:
        print("usage: lz_decomp.py <input.bin> <output.hgr> [offset_decimal]", file=sys.stderr)
        return 2
    data = Path(sys.argv[1]).read_bytes()
    offset = int(sys.argv[3]) if len(sys.argv) > 3 else 0
    fb = decode(data, offset)
    Path(sys.argv[2]).write_bytes(fb.hgr_bytes())
    print(f"wrote {sys.argv[2]} (8192 bytes HGR framebuffer)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
