#!/usr/bin/env bash
# qa_v124_part4.sh — Focused maze + combat + traps test
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/docs/v124_qa_playthrough"
BIN="$ROOT/build/src/wizardry_cht"
ASSETS="$ROOT/assets"

xvfb-run -a -s "-screen 0 1280x720x24" bash -s <<'EOF'
set -u
ROOT="/home/anr2/wizardy-cht/wizardry-cht"
OUT="$ROOT/docs/v124_qa_playthrough"
BIN="$ROOT/build/src/wizardry_cht"
ASSETS="$ROOT/assets"
export WIZ_ASSETS_DIR="$ASSETS"
export SDL_VIDEODRIVER=x11

"$BIN" > /tmp/wiz_stdout4.log 2> /tmp/wiz_stderr4.log &
GAME_PID=$!
sleep 3
WID=$(xdotool search --name "Wizardry" 2>/dev/null | head -1)
echo WID=$WID

snap() { import -window root "$OUT/$1.png" 2>/dev/null; echo "snap: $1"; }
press() { xdotool key --window $WID "$1"; sleep "${2:-0.45}"; }

# Boot → Edge → M → Maze
press space 1.0
press Escape 0.6
press m 1.2
snap "p4_01_maze"

# Force combat IMMEDIATELY
press space 1.5
snap "p4_02_combat_attempt"

# If still in maze, walk and try again
press w 0.4
press space 1.5
snap "p4_03_combat_attempt2"

press w 0.4
press w 0.4
press space 1.5
snap "p4_04_combat_attempt3"

# Walk many steps to trigger random encounter
for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do
  press w 0.18
done
snap "p4_05_walked_20"

# Try space
press space 1.5
snap "p4_06_after_space"

# If we're in combat, try Fight (F) properly
press f 0.6
snap "p4_07_after_F"
press 1 0.5
snap "p4_08_after_1"
press Return 0.5
snap "p4_09_after_return"
sleep 1.0
snap "p4_10_resolve"

# Lots of Returns to flush combat
for i in 1 2 3 4 5 6 7 8 9 10; do
  press Return 0.4
done
snap "p4_11_combat_end"

# Walk more
for i in 1 2 3 4 5 6 7 8 9 10; do
  press w 0.18
done
snap "p4_12_walked_more"

# Try to trigger trap
press d 0.3
for i in 1 2 3 4 5 6 7 8 9 10; do
  press w 0.18
done
snap "p4_13_turn_walk"

press a 0.3
for i in 1 2 3 4 5 6 7 8 9 10; do
  press w 0.18
done
snap "p4_14_walk_again"

# Press space many times to force combat
for i in 1 2 3 4 5; do
  press space 1.0
done
snap "p4_15_after_space_x5"

kill $GAME_PID 2>/dev/null
exit 0
EOF
ls $OUT | grep ^p4_
