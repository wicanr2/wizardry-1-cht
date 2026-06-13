#!/usr/bin/env bash
# qa_v124_part2.sh — Continue QA from town services correctly through maze + combat
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

"$BIN" > /tmp/wiz_stdout2.log 2> /tmp/wiz_stderr2.log &
GAME_PID=$!
sleep 3

WID=$(xdotool search --name "Wizardry" 2>/dev/null | head -1)
if [ -z "$WID" ]; then echo "FAIL window"; exit 1; fi
echo "WID=$WID"

snap() { import -window root "$OUT/$1.png" 2>/dev/null; echo "captured: $1"; }
press() { xdotool key --window $WID "$1" 2>/dev/null; sleep "${2:-0.45}"; }

# Title → tap space → ESC → Edge of Town
press space 1.0
press Escape 0.6
snap "p2_01_edge"

# C → Castle
press c 0.6
snap "p2_02_castle"

# B → shop (Boltac's Trading Post) – within Castle
press b 0.8
snap "p2_03_shop_enter"
# B for buy
press b 0.6
snap "p2_04_shop_buy_menu"
# Try Enter to buy first item
press Return 0.6
snap "p2_05_shop_after_enter"
press Escape 0.5
snap "p2_06_shop_after_esc"
# S for sell
press s 0.6
snap "p2_07_shop_sell"
press Escape 0.5
# I for identify
press i 0.6
snap "p2_08_shop_identify"
press Escape 0.5
# U for uncurse
press u 0.6
snap "p2_09_shop_uncurse"
press Escape 0.5
# P for pool gold
press p 0.6
snap "p2_10_shop_pool"
press Escape 0.5
press Escape 0.5
snap "p2_11_back_to_castle_after_shop"

# C → Temple (Cant's Temple)
press c 0.7
snap "p2_12_temple"
# Try 1..7 service selection
press 1 0.5
snap "p2_13_temple_svc1"
press Escape 0.5
press 2 0.5
snap "p2_14_temple_svc2"
press Escape 0.5
press Escape 0.5
snap "p2_15_back_castle_after_temple"

# T → training grounds? Need to navigate to Edge first.
press Escape 0.5
snap "p2_16_edge_again"
press t 0.7
snap "p2_17_training_grounds"
press Escape 0.5

# M → Maze
press m 1.2
snap "p2_18_maze_b1f"

# Walk W and turn
press w 0.5
snap "p2_19_maze_step_w"
press w 0.5
press w 0.5
snap "p2_20_maze_step_w3"
press d 0.5
snap "p2_21_maze_turn_d"
press w 0.5
press w 0.5
snap "p2_22_maze_step_after_turn"
press a 0.4
press a 0.4
snap "p2_23_maze_face_other_dir"

# Toggle automap
press m 0.5
snap "p2_24_maze_automap_off"
press m 0.5
snap "p2_25_maze_automap_on"

# Force combat
press space 1.5
snap "p2_26_combat_start"

# Try [S] Spell
press s 0.7
snap "p2_27_combat_spell_picker"

# Pick first spell with arrow + Enter
press Return 0.6
snap "p2_28_combat_spell_chosen"

# Pick target with Enter
press Return 0.6
snap "p2_29_combat_spell_target_chosen"

# Confirm whole-party turn submission
press Return 0.8
sleep 1.0
snap "p2_30_combat_resolving"
press space 0.8
press space 0.8
snap "p2_31_combat_step2"
press space 0.8
press space 0.8
snap "p2_32_combat_step3"

# Maybe still in combat? Try Fight
press f 0.5
press Return 0.5
press Return 0.5
press Return 0.8
sleep 1.0
snap "p2_33_combat_after_fight"
press space 0.6
press space 0.6
press space 0.6
snap "p2_34_combat_maybe_over"

# Walk for traps
for i in 1 2 3 4 5 6 7 8 9 10; do
  press w 0.25
done
snap "p2_35_maze_walked_10x"
press d 0.3
for i in 1 2 3 4 5 6 7 8; do
  press w 0.25
done
snap "p2_36_maze_walked_more"

# Open camp (Camp = ESC in maze? Or 'c'?)
press c 0.6
snap "p2_37_camp_or_what"
press Escape 0.5
snap "p2_38_after_esc"

# Try Tab for camp
press Tab 0.6
snap "p2_39_tab_pressed"
press Escape 0.5

# F1 help
press F1 0.6
snap "p2_40_F1_help"
press Escape 0.5

# F2 ???
press F2 0.6
snap "p2_41_F2"
press Escape 0.5

kill $GAME_PID 2>/dev/null
wait 2>/dev/null
exit 0
EOF
RC=$?
echo
ls "$OUT" | grep ^p2_
exit $RC
