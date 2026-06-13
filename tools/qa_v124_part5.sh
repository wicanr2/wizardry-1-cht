#!/usr/bin/env bash
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
"$BIN" > /tmp/wiz_stdout5.log 2> /tmp/wiz_stderr5.log &
GAME_PID=$!
sleep 3
WID=$(xdotool search --name "Wizardry" 2>/dev/null | head -1)
echo WID=$WID

snap() { import -window root "$OUT/$1.png" 2>/dev/null; echo "snap: $1"; }
press() { xdotool key --window $WID "$1"; sleep "${2:-0.45}"; }

press space 1.0
press Escape 0.6
press m 1.2
snap "p5_01_maze"

press space 1.5
snap "p5_02_combat_start"

# Char 1 (阿洛札 fighter): F→Return (auto-pick group 1)
press f 0.5
press Return 0.5
snap "p5_03_char1_done"

# Char 2 (艾琳 mage): S→spell picker, Pick HALITO (first damage)
press s 0.6
snap "p5_04_spell_picker"
# Spell picker — try arrow + Return
press Down 0.3
press Return 0.6
snap "p5_05_spell_picked"
press Return 0.5
snap "p5_06_target_picked"
sleep 0.5
snap "p5_07_after_eirin"

# Char 3 (布蘭達 thief): F
press f 0.5
press Return 0.5
snap "p5_08_char3"

# Char 4 (西里爾 priest): S then DIOS heal
press s 0.5
snap "p5_09_priest_spell"
press Return 0.5
snap "p5_10_priest_pick"
press Return 0.5
sleep 0.5
snap "p5_11_priest_done"

# Char 5 (德蕾達 bishop): F (back row, will fail melee?)
press f 0.5
press Return 0.5
snap "p5_12_char5"

# Char 6 (艾蘿溫 samurai): F (back row)
press f 0.5
press Return 0.5
snap "p5_13_char6"

# Resolve round
sleep 1.5
snap "p5_14_round_done"

# space through resolution
for i in 1 2 3 4 5 6 7 8 9 10; do
  press space 0.4
done
snap "p5_15_after_spaces"

# Press Return through any prompts
for i in 1 2 3 4 5; do
  press Return 0.4
done
snap "p5_16_after_returns"

# Try fight again next round
press f 0.5
press Return 0.5
press f 0.5
press Return 0.5
press f 0.5
press Return 0.5
press f 0.5
press Return 0.5
press f 0.5
press Return 0.5
press f 0.5
press Return 0.5
sleep 2
snap "p5_17_round2"

for i in 1 2 3 4 5 6 7 8 9 10; do
  press space 0.4
done
snap "p5_18_more_spaces"

# Force another combat to test spell slot exhaustion
for i in 1 2 3 4 5; do
  press w 0.3
done
press space 1.5
snap "p5_19_combat3"

# Cast HALITO three times in a row to deplete mage
press s 0.5
press Down 0.3
press Return 0.5
press Return 0.5
sleep 0.7
snap "p5_20_halito_cast"

kill $GAME_PID 2>/dev/null
exit 0
EOF
ls $OUT | grep ^p5_
