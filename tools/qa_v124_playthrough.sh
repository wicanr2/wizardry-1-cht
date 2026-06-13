#!/usr/bin/env bash
# qa_v124_playthrough.sh — Comprehensive QA driver for Wizardry I CHT v1.24.
# Captures docs/v124_qa_playthrough/NN_label.png at every checkpoint.

set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$ROOT/docs/v124_qa_playthrough"
BIN="$ROOT/build/src/wizardry_cht"
ASSETS="$ROOT/assets"

mkdir -p "$OUT"
rm -f "$OUT"/*.png 2>/dev/null

xvfb-run -a -s "-screen 0 1280x720x24" bash -s <<'EOF'
set -u
ROOT="/home/anr2/wizardy-cht/wizardry-cht"
OUT="$ROOT/docs/v124_qa_playthrough"
BIN="$ROOT/build/src/wizardry_cht"
ASSETS="$ROOT/assets"

export WIZ_ASSETS_DIR="$ASSETS"
export SDL_VIDEODRIVER=x11

"$BIN" > /tmp/wiz_stdout.log 2> /tmp/wiz_stderr.log &
GAME_PID=$!
sleep 3

WID=$(xdotool search --name "Wizardry" 2>/dev/null | head -1)
if [ -z "$WID" ]; then
    echo "FAIL: could not locate Wizardry window" >&2
    kill $GAME_PID 2>/dev/null
    exit 1
fi
echo "Game window: $WID"

snap() {
    local name="$1"
    import -window root "$OUT/$name.png" 2>/dev/null
    echo "captured: $name"
}

press() {
    xdotool key --window $WID "$1" 2>/dev/null
    sleep "${2:-0.45}"
}

key_seq() {
    for k in "$@"; do
        xdotool key --window $WID "$k" 2>/dev/null
        sleep 0.3
    done
}

# =========================================================
# PHASE 1 — Boot + chrome (title + F3/F4 cycling)
# =========================================================
snap "01_title_default"

# F4: zh-TW -> EN
press F4 0.6
snap "02_title_F4_english"

# F4: EN -> JP
press F4 0.6
snap "03_title_F4_japanese"

# F4: JP -> zh-TW
press F4 0.6
snap "04_title_F4_zhtw_again"

# F3: cycle theme (capture each)
press F3 0.6
snap "05_title_F3_theme2"
press F3 0.6
snap "06_title_F3_theme3"
press F3 0.6
snap "07_title_F3_theme4"
press F3 0.6
snap "08_title_F3_theme5"
press F3 0.6
snap "09_title_F3_back"

# Test save slot row (try 1..5 selection — they may not have saves yet but row should highlight)
press 2 0.4
snap "10_title_slot2"
press 1 0.4
snap "11_title_slot1"

# Press space/enter to advance from title to intro/edge
press space 1.0
snap "12_after_title_advance"

# Try ESC to land on Edge of Town
press Escape 0.6
snap "13_edge_of_town"

# =========================================================
# PHASE 2 — Town services
# =========================================================
press c 0.6
snap "14_castle_menu"

# G — Tavern
press g 0.7
snap "15_tavern_party"

# Try Shift+Down swap on party order (v1.23)
press Down 0.3
press Down 0.3
xdotool keydown --window $WID shift 2>/dev/null
sleep 0.2
xdotool key --window $WID Down 2>/dev/null
sleep 0.3
xdotool keyup --window $WID shift 2>/dev/null
sleep 0.4
snap "16_tavern_shift_down"

# ESC back to castle
press Escape 0.5
snap "17_castle_after_tavern"

# A — Inn
press a 0.7
snap "18_inn_menu"

# Try room 5 (royal suite) for first slot
press 1 0.5
snap "19_inn_select_member"
press 5 0.8
snap "20_inn_room5"
press space 0.6
snap "21_inn_rested"

# ESC back
press Escape 0.5
press Escape 0.5
snap "22_castle_after_inn"

# B — Boltac's Trading Post
press b 0.7
snap "23_shop_menu"

# Try B for buy
press b 0.6
snap "24_shop_buy"
press Escape 0.5

# Try I for identify (or whatever maps)
press i 0.5
snap "25_shop_identify"
press Escape 0.5
press Escape 0.5
snap "26_castle_after_shop"

# C — Temple
press c 0.7
snap "27_temple_menu"

# Try the 7 services prompt
press 1 0.5
snap "28_temple_svc1"
press Escape 0.4
press 7 0.5
snap "29_temple_svc7"
press Escape 0.4
press Escape 0.4
snap "30_castle_after_temple"

# Try training grounds (T or E?)
press e 0.7
snap "31_training_grounds"
press Escape 0.5

# ESC to Edge
press Escape 0.6
snap "32_edge_again"

# =========================================================
# PHASE 3 — Maze + compass + auto-map + combat + traps
# =========================================================
press m 1.2
snap "33_maze_b1f_entry"

# Walk forward
press w 0.6
snap "34_maze_step1"
press w 0.5
snap "35_maze_step2"

# Turn right (D)
press d 0.5
snap "36_maze_turn_right"

# Turn left (A)
press a 0.4
snap "37_maze_turn_left"

# Turn left again — face West
press a 0.4
snap "38_maze_face_west"

# Toggle automap M
press m 0.5
snap "39_maze_automap_toggled"
press m 0.5
snap "40_maze_automap_back"

# DUMAPIC: open camp first? In Wizardry, DUMAPIC is cast from camp.
# Press Escape/c for camp? Look at hotkeys later.
# Press space to force encounter
press space 1.5
snap "41_combat_start"

# Try [S] spell
press s 0.7
snap "42_combat_spell_picker"

# Pick HALITO (top-left of grid). Press 1?
press 1 0.6
snap "43_combat_spell_chosen"

# Pick target
press 1 0.6
snap "44_combat_spell_target"

# Wait a bit and resolve
sleep 1.0
snap "45_combat_after_cast"

# Press space/return through combat
press space 0.8
snap "46_combat_continue"
press space 0.8
snap "47_combat_continue2"
press space 0.8
snap "48_combat_continue3"

# Try [F] fight
press f 0.6
snap "49_combat_fight"
press 1 0.5
snap "50_combat_fight_target"
press space 0.8
snap "51_combat_after_fight"
sleep 0.8
press space 0.6
press space 0.6
snap "52_after_combat"

# Back in maze, walk around to maybe find a trap
for i in 1 2 3 4 5 6 7 8; do
  press w 0.3
done
snap "53_maze_walked_more"

# Try to trigger DUMAPIC. Look for camp menu via 'C' key
press c 0.7
snap "54_camp_menu"

# Try DUMAPIC option - usually via spell list
press s 0.6
snap "55_camp_spell_list"

# Try first spell typed name
press d 0.4
snap "56_camp_d_key"
press Escape 0.5
press Escape 0.5
snap "57_back_maze"

# Walk more to trigger something
for i in 1 2 3 4 5 6 7 8 9 10; do
  press w 0.2
done
snap "58_maze_more_walking"

# Turn around 180
press d 0.3
press d 0.3
snap "59_maze_turned_around"

# Walk back
for i in 1 2 3 4 5 6 7 8; do
  press w 0.2
done
snap "60_maze_walked_back"

# Force another combat for spell slot check
press space 1.5
snap "61_combat2_start"
press s 0.6
snap "62_combat2_spell_picker"
press Escape 0.5
press f 0.5
press 1 0.5
press space 0.8
sleep 0.5
press space 0.5
press space 0.5
snap "63_combat2_after"

# Try to exit maze via stairs up (which only works on B1F up to town)
# Press U for up stairs maybe
press u 0.5
snap "64_maze_u_key"

# Press E for elevator? Or end?
press Escape 0.5
snap "65_maze_esc"

kill $GAME_PID 2>/dev/null
wait 2>/dev/null
exit 0
EOF

RC=$?
echo
echo "=== Captured files ==="
ls "$OUT" | head -80
echo "Total: $(ls $OUT | wc -l)"
exit $RC
