#!/usr/bin/env bash
# record_playthrough.sh — End-to-end golden-path screenshot recorder for
# Wizardry I CHT. Runs the binary inside Xvfb, injects keystrokes via
# xdotool, and grabs each scene with ImageMagick `import`.
#
# Output: docs/v124_playthrough/NN_scene.png  (1280x720)
#         docs/v124_playthrough_montage.png   (4x3 montage)
#
# Pre-reqs: xvfb, xdotool, imagemagick. All apt-installable.
#
# Usage:
#   tools/record_playthrough.sh
#
# Notes on navigation gotchas (encoded into the key sequence below):
#   - Edge of Town menu: ESC = Leave Game (quits) — never press ESC there.
#   - Castle menu: ESC returns to Edge of Town.
#   - Intro overlay: ESC cancels and lands on Edge of Town.
#   - The `m` key inside the maze toggles the auto-map overlay (state.show_auto_map).
#   - The `space` key inside the maze fires a forced combat encounter.

set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/docs/v124_playthrough"
BIN="$ROOT/build/src/wizardry_cht"
ASSETS="$ROOT/assets"

mkdir -p "$OUT"

xvfb-run -a -s "-screen 0 1280x720x24" bash -s <<EOF
set -u
export WIZ_ASSETS_DIR="$ASSETS"
export SDL_VIDEODRIVER=x11

"$BIN" > /tmp/wiz_stdout.log 2> /tmp/wiz_stderr.log &
GAME_PID=\$!
sleep 3

WID=\$(xdotool search --name "Wizardry" | head -1)
if [ -z "\$WID" ]; then
    echo "FAIL: could not locate Wizardry window" >&2
    kill \$GAME_PID 2>/dev/null
    exit 1
fi
echo "Game window: \$WID"

snap() {
    local name="\$1"
    import -window root "$OUT/\$name.png"
    echo "captured: \$name"
}

press() {
    xdotool key --window \$WID "\$1" 2>/dev/null
    sleep 0.5
}

# ---- 01: Title screen ----
snap "01_title"

# ---- 02: Intro overlay page 1 ----
press space
snap "02_intro_p1"

# ---- 03: Intro overlay page 3 (advance a couple more pages for variety) ----
press space
press space
snap "03_intro_p3"

# ---- 04: Edge of Town (ESC from intro lands here) ----
press Escape
snap "04_edge_of_town"

# ---- 05: Castle menu (C from Edge of Town) ----
press c
snap "05_castle"

# ---- 06: Gilgamesh's Tavern (G from Castle) — demo party of 6 ----
press g
snap "06_tavern"

# ---- 07: Boltac's Trading Post (ESC → Castle → B) ----
press Escape
press b
snap "07_shop"

# ---- 08: Cant's Temple (ESC → Castle → C) ----
press Escape
press c
snap "08_temple"

# ---- 09: Adventurer's Inn (ESC → Castle → A) ----
press Escape
press a
snap "09_inn"

# ---- 10: Maze B1F (ESC → Castle → E → Edge of Town → M) ----
press Escape
# Now at Castle. Cursor moves with menu hotkey E.
press e
sleep 0.4
# Now at Edge of Town. M = Maze.
press m
sleep 1.0
snap "10_maze_b1f"

# ---- 11: Maze with auto-map toggled OFF (compass overlay, larger 3D view) ----
press m
sleep 0.5
snap "11_maze_3d_compass"

# ---- 12: Combat — SPACE forces an encounter ----
press space
sleep 1.5
snap "12_combat"

kill \$GAME_PID 2>/dev/null
wait 2>/dev/null
exit 0
EOF

RC=$?
echo
echo "=== Captured files ==="
ls -la "$OUT"
echo
echo "=== Building montage ==="
montage "$OUT"/*.png -tile 4x3 -geometry 320x180+8+8 \
    -background '#0a0a0a' "$ROOT/docs/v124_playthrough_montage.png"
ls -la "$ROOT/docs/v124_playthrough_montage.png"

exit $RC
