#!/usr/bin/env bash
# Part 7: Camp + DUMAPIC + walks (no ESC at Edge)
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
"$BIN" > /tmp/wiz_stdout7.log 2> /tmp/wiz_stderr7.log &
GAME_PID=$!
sleep 3
WID=$(xdotool search --name "Wizardry" 2>/dev/null | head -1)
echo WID=$WID
snap() { import -window root "$OUT/$1.png" 2>/dev/null; echo "snap: $1"; }
press() { xdotool key --window $WID "$1"; sleep "${2:-0.45}"; }

press space 1.0
press Escape 0.6
press m 1.2
snap "p7_01_maze"

# Enter camp directly
press c 0.6
snap "p7_02_camp_menu"

# Try spell from camp
press s 0.5
snap "p7_03_spell_picker"

# Navigate to DUMAPIC — it's position 3 in row 1: HALITO MOGREF KATINO DUMAPIC
press Right 0.3
press Right 0.3
press Right 0.3
snap "p7_04_picker_at_dumapic"
press Return 0.5
snap "p7_05_dumapic_chosen"
# Hopefully shows position
sleep 1
snap "p7_06_dumapic_result"
press space 0.5
snap "p7_07_after_space"

# Try LATUMAPIC (priest)
press s 0.5
snap "p7_08_picker_again"
# Try CALFO — it should be a priest spell, but we're a mage caster?
# Let me arrow around
for i in 1 2 3 4 5 6 7 8 9; do press Down 0.2; done
snap "p7_09_picker_bottom"
press Escape 0.5

# Try inventory I
press i 0.5
snap "p7_10_inventory"
press Escape 0.5

# Try Equipment E
press e 0.5
snap "p7_11_equipment_or_export"
press Escape 0.5

# Try Reorder R
press r 0.5
snap "p7_12_reorder"
press Escape 0.5

# Quit camp back to maze
press Escape 0.5
snap "p7_13_back_maze"

# Walk 30 steps to find traps / encounters
for i in $(seq 1 30); do
  press w 0.15
done
snap "p7_14_walked_30"

press d 0.3
for i in $(seq 1 25); do
  press w 0.15
done
snap "p7_15_walked_25"

press a 0.3
press a 0.3
for i in $(seq 1 20); do
  press w 0.15
done
snap "p7_16_walked_20"

# Look at floor for stairs
press Return 0.5
snap "p7_17_after_return"

# Try going up — Return on stairs
press Return 0.5
snap "p7_18_after_return2"

# F5 = automap toggle?
press F5 0.5
snap "p7_19_F5"

# F1 in maze
press F1 0.5
snap "p7_20_F1"

kill $GAME_PID 2>/dev/null
exit 0
EOF
ls $OUT | grep ^p7_
