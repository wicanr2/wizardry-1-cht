#!/usr/bin/env bash
# Fetch the CJK font(s) required at runtime.
# Tries the system-installed Noto Sans CJK first (Linux);
# falls back to downloading from Google Fonts release.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
FONT_DIR="$ROOT_DIR/assets/fonts"
TARGET="$FONT_DIR/NotoSansCJK-Regular.ttc"

mkdir -p "$FONT_DIR"

if [[ -f "$TARGET" ]]; then
    echo "Font already present: $TARGET"
    exit 0
fi

# 1) Try system path (most Linux distros ship Noto CJK)
for path in \
    /usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc \
    /usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc \
    /usr/share/fonts/TTF/NotoSansCJK-Regular.ttc \
    /Library/Fonts/NotoSansCJK-Regular.ttc \
    "C:/Windows/Fonts/NotoSansCJK-Regular.ttc"
do
    if [[ -f "$path" ]]; then
        cp -v "$path" "$TARGET"
        exit 0
    fi
done

# 2) Download from upstream release
echo "System font not found. Downloading from Google Noto Fonts release..."
URL="https://github.com/notofonts/noto-cjk/raw/main/Sans/OTC/NotoSansCJK-Regular.ttc"
if command -v curl >/dev/null; then
    curl -L -o "$TARGET" "$URL"
elif command -v wget >/dev/null; then
    wget -O "$TARGET" "$URL"
else
    echo "ERROR: need curl or wget to download the font."
    echo "Please put NotoSansCJK-Regular.ttc into $FONT_DIR manually."
    exit 1
fi

echo "Font installed: $TARGET"
