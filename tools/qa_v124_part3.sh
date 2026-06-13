#!/usr/bin/env bash
# qa_v124_part3.sh — Focus on maze + combat + traps + spell slots
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/docs/v124_qa_playthrough"
BIN="$ROOT/build/src/wizardry_cht"
ASSETS="$ROOT/assets"
mkdir -p "$OUT"

xvfb-run -a -s "-screen 0 1280x720x24" bash -s <<'EOF'
set -u
ROOT="/home/anr2/wizardy-cht/wizardry-cht"
OUT="$ROOT/docs/v124_qa_playthrough"
BIN="$ROOT/build/src/wizardry_cht"
ASSETS="$ROOT/assets"
export WIZ_ASSETS_DIR="$ASSETS"
export SDL_VIDEODRIVER=x11

"$BIN" > /tmp/wiz_stdout3.log 2> /tmp/wiz_stderr3.log &
GAME_PID=$!
sleep 3
WID=$(xdotool search --name "Wizardry" 2>/dev/null | head -1)
[ -z "$WID" ] && { echo "no window"; exit 1; }
echo WID=$WID

snap() { import -window root "$OUT/$1.png" 2>/dev/null; echo "snap: $1"; }
press() { xdotool key --window $WID "$1"; sleep "${2:-0.45}"; }

# Boot → Edge → M → Maze (single ESC max!)
press space 1.0
press Escape 0.6
snap "p3_01_edge"
press m 1.2
snap "p3_02_maze_b1f_initial"

# Maze controls: walk W (forward)
press w 0.5
snap "p3_03_step1"
press w 0.5
press w 0.5
snap "p3_04_step3"

# Turn right (d) — face East
press d 0.4
snap "p3_05_face_east"
press w 0.4
snap "p3_06_east_step1"
press w 0.4
press w 0.4
snap "p3_07_east_step3"

# Turn back left twice (face West)
press a 0.4
press a 0.4
snap "p3_08_face_west"

# Press F1 help in maze
press F1 0.6
snap "p3_09_F1_help"
press Escape 0.5
snap "p3_10_after_help_close"

# Press F2
press F2 0.6
snap "p3_11_F2"
press Escape 0.5

# Toggle automap with M
press m 0.4
snap "p3_12_automap_off_q"
press m 0.4
snap "p3_13_automap_on_q"

# Try to enter Camp (the docs imply Tab or C)
press Tab 0.6
snap "p3_14_tab"
press Escape 0.5
snap "p3_15_post_tab_esc"

# Press 'c' = Camp?
press c 0.6
snap "p3_16_c_key"

# If we're in camp, try a spell
press s 0.5
snap "p3_17_camp_spell_menu"

# Try DUMAPIC?
press d 0.4
snap "p3_18_camp_d"
press Return 0.5
snap "p3_19_camp_enter"
press Escape 0.5
press Escape 0.5
snap "p3_20_back_maze"

# Force combat
press space 1.5
snap "p3_21_combat_start"

# Look at combat carefully — what does prompt say?
snap "p3_22_combat_again"

# F = fight
press f 0.5
snap "p3_23_combat_F_fight"
press Return 0.5
snap "p3_24_combat_after_F"

# Continue
press Return 0.5
press Return 0.5
press Return 0.5
snap "p3_25_combat_progress"

# Try arrow + enter
press Down 0.3
press Return 0.5
snap "p3_26_after_enter"

# Try S spell
press s 0.5
snap "p3_27_spell_menu"

# Pick spell with arrow keys
press Down 0.3
press Down 0.3
press Return 0.5
snap "p3_28_spell_picked"

press Return 0.5
sleep 0.7
snap "p3_29_after_pick"
press Return 0.5
sleep 0.7
snap "p3_30_after_resolve"

press space 0.6
press space 0.6
snap "p3_31_after_space"

# Walk lots to find a trap
for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15; do
  press w 0.2
done
snap "p3_32_walked_15"

press d 0.3
for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15; do
  press w 0.2
done
snap "p3_33_walked_30"

press d 0.3
for i in 1 2 3 4 5 6 7 8 9 10; do
  press w 0.2
done
snap "p3_34_walked_more"

kill $GAME_PID 2>/dev/null
wait 2>/dev/null
exit 0
EOF
ls $OUT | grep ^p3_
