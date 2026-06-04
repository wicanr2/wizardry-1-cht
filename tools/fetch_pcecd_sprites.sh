#!/usr/bin/env bash
# Fetch PCE-CD monster sprites from wizardry.wiki.gg
# (CC-BY-SA 4.0, see docs/ASSETS.md for attribution requirements).
#
# Each sprite is 64×96px PNG, ~1-5KB. Total ~94 files, ~300KB.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
OUT_DIR="$ROOT_DIR/assets/sprites/monsters_pcecd"
UA="Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36"

mkdir -p "$OUT_DIR"
cd "$OUT_DIR"

# Full list of PCE-CD monster sprite filenames (94 entries) discovered via
# the wiki.gg category page. Stored canonical names so this script doesn't
# need to re-scrape the category index on every run.
SPRITES=(
    "PCECD_MS_Arch_Mage.png"
    "PCECD_MS_Attack_Dog.png"
    "PCECD_MS_Bishop.png"
    "PCECD_MS_Bleeb.png"
    "PCECD_MS_Boring_Beetle.png"
    "PCECD_MS_Bubbly_Slime.png"
    "PCECD_MS_Buccaneer.png"
    "PCECD_MS_Bushwacker.png"
    "PCECD_MS_Capybara.png"
    "PCECD_MS_Chimera.png"
    "PCECD_MS_Cleric.png"
    "PCECD_MS_Coyote.png"
    "PCECD_MS_Creeping_Coins.png"
    "PCECD_MS_Creeping_Crud.png"
    "PCECD_MS_Dragon_Fly.png"
    "PCECD_MS_Dragon_Zombie.png"
    "PCECD_MS_Earth_Giant.png"
    "PCECD_MS_Fire_Dragon.png"
    "PCECD_MS_Fire_Giant.png"
    "PCECD_MS_Frost_Giant.png"
    "PCECD_MS_Gargoyle.png"
    "PCECD_MS_Gas_Cloud.png"
    "PCECD_MS_Gas_Dragon.png"
    "PCECD_MS_Gaze_Hound.png"
    "PCECD_MS_Ghost.png"
    "PCECD_MS_Giant_Mantis.png"
    "PCECD_MS_Giant_Spider.png"
    "PCECD_MS_Giant_Toad.png"
    "PCECD_MS_Gnome.png"
    "PCECD_MS_Greater_Demon.png"
    "PCECD_MS_Grave_Mist.png"
    "PCECD_MS_Hatamoto.png"
    "PCECD_MS_High_Corsair.png"
    "PCECD_MS_High_Ninja.png"
    "PCECD_MS_High_Priest.png"
    "PCECD_MS_High_Wizard.png"
    "PCECD_MS_Highwayman.png"
    "PCECD_MS_Huge_Spider.png"
    "PCECD_MS_Kobold.png"
    "PCECD_MS_Lesser_Demon.png"
    "PCECD_MS_Level_10_Mage.png"
    "PCECD_MS_Level_10_Priest.png"
    "PCECD_MS_Level_3_Mage.png"
    "PCECD_MS_Level_3_Priest.png"
    "PCECD_MS_Level_5_Fighter.png"
    "PCECD_MS_Level_5_Mage.png"
    "PCECD_MS_Level_5_Ninja.png"
    "PCECD_MS_Level_5_Priest.png"
    "PCECD_MS_Level_5_Thief.png"
    "PCECD_MS_Level_7_Fighter.png"
    "PCECD_MS_Level_7_Mage.png"
    "PCECD_MS_Level_7_Ninja.png"
    "PCECD_MS_Level_7_Priest.png"
    "PCECD_MS_Level_7_Thief.png"
    "PCECD_MS_Level_8_Fighter.png"
    "PCECD_MS_Level_8_Mage.png"
    "PCECD_MS_Level_8_Ninja.png"
    "PCECD_MS_Level_8_Priest.png"
    "PCECD_MS_Level_8_Thief.png"
    "PCECD_MS_Lifestealer.png"
    "PCECD_MS_Lvl_1_Fighter.png"
    "PCECD_MS_Lvl_1_Mage.png"
    "PCECD_MS_Lvl_1_Ninja.png"
    "PCECD_MS_Lvl_1_Priest.png"
    "PCECD_MS_Lvl_1_Thief.png"
    "PCECD_MS_Lvl_2_Mage.png"
    "PCECD_MS_Lvl_2_Priest.png"
    "PCECD_MS_Major_Daimyo.png"
    "PCECD_MS_Manticore.png"
    "PCECD_MS_Master_Ninja.png"
    "PCECD_MS_Master_Thief.png"
    "PCECD_MS_Medusalizard.png"
    "PCECD_MS_Murphys_Ghost.png"
    "PCECD_MS_No_See_Um.png"
    "PCECD_MS_Ogre.png"
    "PCECD_MS_Ogre_Lord.png"
    "PCECD_MS_Orc.png"
    "PCECD_MS_Page.png"
    "PCECD_MS_Poison_Giant.png"
    "PCECD_MS_Priestess.png"
    "PCECD_MS_Rogue.png"
    "PCECD_MS_Shade.png"
    "PCECD_MS_Sorcerer.png"
    "PCECD_MS_Spirit.png"
    "PCECD_MS_Swordsman.png"
    "PCECD_MS_The_High_Wizard.png"
    "PCECD_MS_Thief.png"
    "PCECD_MS_Troll.png"
    "PCECD_MS_Undead_Kobold.png"
    "PCECD_MS_Vampire.png"
    "PCECD_MS_Vampire_Lord.png"
    "PCECD_MS_Vorpal_Bunny.png"
    "PCECD_MS_Werdna.png"
    "PCECD_MS_Were_Bear.png"
    "PCECD_MS_Will_O_Wisp.png"
    "PCECD_MS_Wyvern.png"
    "PCECD_MS_Zombie.png"
)

ok=0
fail=0
for fn in "${SPRITES[@]}"; do
    if [[ -f "$fn" ]]; then
        ok=$((ok + 1))
        continue
    fi
    url="https://wizardry.wiki.gg/images/$fn"
    if curl -sLf -A "$UA" "$url" -o "$fn" 2>/dev/null && \
       [[ "$(file -b --mime-type "$fn")" == "image/png" ]]; then
        printf "  ✓ %s\n" "$fn"
        ok=$((ok + 1))
    else
        printf "  ✗ %s\n" "$fn"
        rm -f "$fn"
        fail=$((fail + 1))
    fi
    sleep 0.2  # be polite to the CDN
done

echo ""
echo "Downloaded: $ok ok, $fail fail (out of ${#SPRITES[@]})"
echo "Output:     $OUT_DIR"

cat > "$OUT_DIR/LICENSE.txt" <<'LIC'
These sprites are sourced from wizardry.wiki.gg and licensed under
Creative Commons Attribution-ShareAlike 4.0 International (CC-BY-SA 4.0):
  https://creativecommons.org/licenses/by-sa/4.0/

Original assets are from the PC-Engine CD release of Wizardry: Proving
Grounds of the Mad Overlord (1991, ASCII Entertainment / Sir-tech).
Wizardry is a trademark of Sir-tech Software / Drecom.

This directory is excluded from the main repo (.gitignore). Run this
script (tools/fetch_pcecd_sprites.sh) to repopulate it.
LIC
echo "Wrote $OUT_DIR/LICENSE.txt"
