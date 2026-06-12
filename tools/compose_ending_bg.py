#!/usr/bin/env python3
"""
Compose a 1280x720 PCE-CD style ending background for the Werdna-defeat scene.

Layers (top to bottom in z order):
  - Base: black with a subtle radial purple → indigo glow centred mid-screen
  - Upper third: castle-aerial OVA frame (640x480) scaled to 1280x540, faded
    + crossed under a stone-blue overlay so it reads as "morning over Trebor's
    keep" rather than a screenshot of a different show
  - Mid: golden glow disc representing the recovered Amulet of Werdna
  - Lower third: defeated Werdna sprite, greyed and at 50% alpha, slightly
    skewed so he reads as "slumped on the throne"
  - Vignette: corner fade so menu/text overlays read

Output: assets/themes/pcecd/ending/background.png

Run from repo root:  python3 tools/compose_ending_bg.py
"""
import os
from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter, ImageEnhance

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "assets/themes/pcecd/ending/background.png"
CASTLE = ROOT / "docs/ova_frames/01_castle_aerial.jpg"
WERDNA = ROOT / "assets/sprites/monsters_pcecd/PCECD_MS_Werdna.png"

W, H = 1280, 720

def radial_gradient(size, inner, outer):
    """Returns an RGBA image with a radial gradient from inner→outer."""
    img = Image.new("RGBA", size)
    cx, cy = size[0] // 2, int(size[1] * 0.45)
    max_r = max(cx, cy, size[0] - cx, size[1] - cy)
    px = img.load()
    for y in range(size[1]):
        for x in range(size[0]):
            d = ((x - cx) ** 2 + (y - cy) ** 2) ** 0.5
            t = min(1.0, d / max_r)
            r = int(inner[0] * (1 - t) + outer[0] * t)
            g = int(inner[1] * (1 - t) + outer[1] * t)
            b = int(inner[2] * (1 - t) + outer[2] * t)
            px[x, y] = (r, g, b, 255)
    return img

def main():
    OUT.parent.mkdir(parents=True, exist_ok=True)
    base = radial_gradient((W, H), inner=(54, 30, 90), outer=(8, 6, 18))

    # Layer 1: castle aerial in the upper third, faded.
    if CASTLE.exists():
        castle = Image.open(CASTLE).convert("RGBA").resize((W, 540), Image.LANCZOS)
        # Boost golden hour feel
        castle = ImageEnhance.Color(castle).enhance(1.25)
        castle = ImageEnhance.Brightness(castle).enhance(0.85)
        # Soft fade at the bottom so it blends into the dark base.
        mask = Image.new("L", castle.size, 0)
        d = ImageDraw.Draw(mask)
        for y in range(castle.size[1]):
            # Top 60% opaque, bottom 40% fades to transparent.
            if y < castle.size[1] * 0.55:
                d.line([(0, y), (castle.size[0], y)], fill=215)
            else:
                t = (y - castle.size[1] * 0.55) / (castle.size[1] * 0.45)
                d.line([(0, y), (castle.size[0], y)], fill=int(215 * (1 - t)))
        castle.putalpha(mask)
        base.alpha_composite(castle, (0, 0))

    # Layer 2: radial gold glow ~ amulet, centred mid-screen.
    glow = Image.new("RGBA", (W, H))
    gd = ImageDraw.Draw(glow)
    cx, cy = W // 2, int(H * 0.55)
    for r, alpha in [(280, 18), (220, 30), (160, 55), (110, 90), (60, 160)]:
        gd.ellipse([cx - r, cy - r, cx + r, cy + r],
                   fill=(255, 220, 120, alpha))
    glow = glow.filter(ImageFilter.GaussianBlur(40))
    base.alpha_composite(glow, (0, 0))

    # Inner amulet disc — solid gold.
    am = Image.new("RGBA", (W, H))
    ad = ImageDraw.Draw(am)
    ad.ellipse([cx - 55, cy - 55, cx + 55, cy + 55],
               fill=(240, 200, 90, 255))
    ad.ellipse([cx - 50, cy - 50, cx + 50, cy + 50],
               outline=(180, 130, 30, 255), width=3)
    # "W" rune in centre
    ad.line([(cx - 25, cy - 22), (cx - 12, cy + 22), (cx, cy - 10),
             (cx + 12, cy + 22), (cx + 25, cy - 22)],
            fill=(70, 30, 0, 255), width=4)
    base.alpha_composite(am, (0, 0))

    # Layer 3: defeated Werdna sprite, greyed + low alpha, lower-left.
    if WERDNA.exists():
        wer = Image.open(WERDNA).convert("RGBA")
        # Scale up 4x for visibility (sprite is small).
        wer = wer.resize((wer.width * 4, wer.height * 4), Image.NEAREST)
        # Desaturate and dim — he's beaten.
        gray = wer.convert("L").convert("RGBA")
        gray = ImageEnhance.Brightness(gray).enhance(0.5)
        # Re-attach the alpha mask
        gray.putalpha(wer.split()[3])
        # Tint indigo so he reads as "dead/cold".
        tint = Image.new("RGBA", gray.size, (40, 30, 80, 0))
        gray = Image.alpha_composite(gray, tint)
        # Half-transparent overall
        alpha = gray.split()[3].point(lambda v: int(v * 0.55))
        gray.putalpha(alpha)
        base.alpha_composite(gray, (60, H - gray.height - 30))

    # Layer 4: corner vignette so menu/text overlay reads.
    vignette = Image.new("L", (W, H), 255)
    vd = ImageDraw.Draw(vignette)
    for r, alpha in [(700, 0), (640, 60), (560, 130), (480, 200), (400, 240)]:
        vd.ellipse([W // 2 - r, H // 2 - r, W // 2 + r, H // 2 + r], fill=alpha)
    vignette = vignette.filter(ImageFilter.GaussianBlur(50))
    base.putalpha(vignette)
    # Re-composite over true black so vignette darkens corners.
    out = Image.new("RGBA", (W, H), (0, 0, 0, 255))
    out.alpha_composite(base)
    out.convert("RGB").save(OUT, "PNG", optimize=True)
    print(f"Wrote {OUT} ({OUT.stat().st_size // 1024} KiB)")

if __name__ == "__main__":
    main()
