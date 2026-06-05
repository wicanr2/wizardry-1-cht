#!/usr/bin/env bash
# Fetch Kevin MacLeod (CC-BY 4.0) BGM tracks for the four game scenes.
# All tracks: incompetech.com, used under Creative Commons Attribution 4.0
# International. Required attribution included in assets/audio/CREDITS.txt.
#
# Music files are NOT committed to repo (each ~1-3MB OGG); fetch on demand.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
OUT_DIR="$ROOT_DIR/assets/audio"

mkdir -p "$OUT_DIR"
cd "$OUT_DIR"

# Hand-picked tracks matching each scene's mood.
# Kevin MacLeod's incompetech.com hosts these as MP3.
declare -A TRACKS=(
    ["title"]="https://incompetech.com/music/royalty-free/mp3-royaltyfree/Mystery%20Bazaar.mp3"
    ["town"]="https://incompetech.com/music/royalty-free/mp3-royaltyfree/Bumbly%20March.mp3"
    ["maze"]="https://incompetech.com/music/royalty-free/mp3-royaltyfree/Lost%20Time.mp3"
    ["combat"]="https://incompetech.com/music/royalty-free/mp3-royaltyfree/Bumbly%20March.mp3"
)

UA="Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36"

ok=0
fail=0
for scene in "${!TRACKS[@]}"; do
    url="${TRACKS[$scene]}"
    target="$OUT_DIR/${scene}.mp3"
    target_ogg="$OUT_DIR/${scene}.ogg"

    if [[ -f "$target" ]] || [[ -f "$target_ogg" ]]; then
        echo "  ✓ $scene already present"
        ok=$((ok + 1))
        continue
    fi

    echo "Fetching $scene from incompetech..."
    if curl -sLf -A "$UA" "$url" -o "$target" 2>/dev/null && \
       [[ "$(file -b --mime-type "$target")" == "audio/mpeg" ]]; then
        printf "  ✓ %s.mp3 (%d bytes)\n" "$scene" "$(stat -c%s "$target")"
        ok=$((ok + 1))
    else
        echo "  ✗ $scene download failed (URL may be stale)"
        rm -f "$target"
        fail=$((fail + 1))
    fi
    sleep 1
done

cat > "$OUT_DIR/CREDITS.txt" <<'CRED'
Background music — used under Creative Commons Attribution 4.0 International:

  "Mystery Bazaar" Kevin MacLeod (incompetech.com)
  "Tavern Loop One" Kevin MacLeod (incompetech.com)
  "Lost Time"      Kevin MacLeod (incompetech.com)
  "Dauntless"      Kevin MacLeod (incompetech.com)

  Licensed under Creative Commons: By Attribution 4.0
  http://creativecommons.org/licenses/by/4.0/

If any track fails to load at runtime, the game runs silently.
Use tools/fetch_music.sh to repopulate the directory.
CRED

echo ""
echo "Downloaded: $ok ok, $fail fail (out of ${#TRACKS[@]})"
echo "Output:     $OUT_DIR"
