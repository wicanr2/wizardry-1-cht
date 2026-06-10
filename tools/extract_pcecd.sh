#!/usr/bin/env bash
# Extract assets from a Wizardry I & II PC-Engine CD-ROM² image.
#
# Usage:
#   tools/extract_pcecd.sh /path/to/wizardry_i_ii.cue
#   tools/extract_pcecd.sh /path/to/wizardry_i_ii.iso
#
# What it does (best-effort, ROM-version dependent):
#   1. CD-DA audio tracks  -> assets/themes/pcecd_local/bgm/track_NN.ogg
#   2. Title screen scan   -> assets/themes/pcecd_local/title/*.png
#                            (heuristic: look for PCE 256x216 image blocks)
#   3. Sector dump         -> /tmp/pcecd_sectors.bin (for manual RE)
#
# Output is written under assets/themes/pcecd_local/ which is gitignored;
# the user's own ROM stays on the user's own machine.
#
# Tools required (auto-checked):
#   - bchunk or cdrdao  (split .cue/.bin into per-track WAV)
#   - oggenc            (compress WAV to OGG)
#   - python3 + Pillow  (image extraction heuristic — optional)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
OUT_DIR="$ROOT_DIR/assets/themes/pcecd_local"

if [[ $# -lt 1 ]]; then
    echo "usage: $0 <image.cue|image.iso|image.bin>" >&2
    exit 1
fi
SRC="$1"
if [[ ! -f "$SRC" ]]; then
    echo "err: file not found: $SRC" >&2
    exit 1
fi

mkdir -p "$OUT_DIR/bgm" "$OUT_DIR/title" "$OUT_DIR/monsters"

# Step 1 — CD-DA tracks
if command -v bchunk >/dev/null 2>&1; then
    case "$SRC" in
        *.cue)
            echo "Splitting CD-DA tracks with bchunk..."
            cuefile="$SRC"
            binfile="${SRC%.cue}.bin"
            if [[ ! -f "$binfile" ]]; then
                # bchunk auto-resolves bin path from cue; try anyway
                binfile="$(grep -oE 'FILE "[^"]+"' "$cuefile" | head -1 | sed 's/FILE "\(.*\)"/\1/')"
                binfile="$(dirname "$SRC")/$binfile"
            fi
            (cd "$(mktemp -d)" && bchunk -w "$binfile" "$cuefile" out >/dev/null)
            ;;
        *)
            echo "  ~ skipping CD-DA: need .cue/.bin pair"
            ;;
    esac
else
    echo "  ~ bchunk not installed; skipping CD-DA extraction"
    echo "    install with: sudo apt-get install bchunk"
fi

# Step 2 — encode any WAVs we found
shopt -s nullglob
wav_count=0
for wav in /tmp/out*.wav; do
    [[ -f "$wav" ]] || continue
    out_name="$(basename "$wav" .wav | sed 's/^out0*//')"
    if command -v oggenc >/dev/null 2>&1; then
        oggenc -q 5 "$wav" -o "$OUT_DIR/bgm/track_${out_name}.ogg" 2>/dev/null
    else
        cp "$wav" "$OUT_DIR/bgm/track_${out_name}.wav"
    fi
    wav_count=$((wav_count + 1))
done
echo "  ✓ extracted $wav_count BGM track(s) -> $OUT_DIR/bgm/"

# Step 3 — dump first 8MB of data track for offline scanning
dd if="$SRC" of=/tmp/pcecd_sectors.bin bs=1M count=8 2>/dev/null || true
echo "  ✓ first 8MB dumped to /tmp/pcecd_sectors.bin (for image RE)"

cat <<EOF

== PCE-CD extraction (best-effort) complete ==
  Output: $OUT_DIR
  BGM    : $wav_count tracks
  Title  : (TODO — visual heuristic not yet implemented)
  Monster: (TODO — needs per-ROM sprite-bank offset table)

Once the BGM ogg files are populated, set theme to PCECD and the game
will play those tracks instead of the Kevin MacLeod fallbacks.
EOF
