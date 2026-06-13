#!/usr/bin/env bash
# Part 6: temple + camp + DUMAPIC + training
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
"$BIN" > /tmp/wiz_stdout6.log 2> /tmp/wiz_stderr6.log &
GAME_PID=$!
sleep 3
WID=$(xdotool search --name "Wizardry" 2>/dev/null | head -1)
echo WID=$WID
snap() { import -window root "$OUT/$1.png" 2>/dev/null; echo "snap: $1"; }
press() { xdotool key --window $WID "$1"; sleep "${2:-0.45}"; }

press space 1.0
press Escape 0.6
snap "p6_01_edge"

# Test: go to Castle → Temple
press c 0.6
snap "p6_02_castle"
press c 0.8
snap "p6_03_temple"
# Try keys 1-7 for services
press 1 0.6
snap "p6_04_temple_1"
press 2 0.6
snap "p6_05_temple_2"
press 3 0.6
snap "p6_06_temple_3"
press 4 0.6
snap "p6_07_temple_4"
press 5 0.6
snap "p6_08_temple_5"
press 6 0.6
snap "p6_09_temple_6"
press 7 0.6
snap "p6_10_temple_7"
# Try letter keys
press h 0.5
snap "p6_11_temple_h"
press p 0.5
snap "p6_12_temple_p"
press r 0.5
snap "p6_13_temple_r"
press k 0.5
snap "p6_14_temple_k"
press u 0.5
snap "p6_15_temple_u"

# ESC back to Castle
press Escape 0.5
snap "p6_16_castle_again"

# ESC to Edge
press Escape 0.5
snap "p6_17_edge_again"

# T = training grounds (Edge level)
press t 0.7
snap "p6_18_training"
# Try N for new char
press n 0.6
snap "p6_19_train_n"
press Escape 0.5
# Try R
press r 0.5
snap "p6_20_train_r"
press Escape 0.5
snap "p6_21_train_back"

# Back to maze
press Escape 0.5
press m 1.2
snap "p6_22_maze"

# CAMP via C key
press c 0.6
snap "p6_23_camp"

# In camp, test all menu items
press s 0.5
snap "p6_24_camp_S"
press Escape 0.5
press e 0.5
snap "p6_25_camp_E"
press Escape 0.5
press r 0.5
snap "p6_26_camp_R"
press Escape 0.5
press i 0.5
snap "p6_27_camp_I"
press Escape 0.5

# Try spell from camp — S then pick DUMAPIC
press s 0.5
snap "p6_28_spell_picker_camp"
# Hopefully arrow keys can find DUMAPIC
for i in 1 2 3 4 5; do
  press Right 0.2
done
snap "p6_29_picker_right5"
press Return 0.5
snap "p6_30_after_return"
press Return 0.5
sleep 0.7
snap "p6_31_dumapic_result"

# Try CALFO test (priest spell)
press Escape 0.5
press s 0.5
press Down 0.3
press Down 0.3
press Down 0.3
press Down 0.3
for i in 1 2 3 4 5 6 7; do press Right 0.2; done
snap "p6_32_picker_priest"
press Return 0.5
snap "p6_33_priest_chosen"
press Escape 0.5
press Escape 0.5

# Walk 20 steps trying to find trap
for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do
  press w 0.18
done
snap "p6_34_walked"
press d 0.3
for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15; do
  press w 0.18
done
snap "p6_35_more_walked"

kill $GAME_PID 2>/dev/null
exit 0
EOF
ls $OUT | grep ^p6_
