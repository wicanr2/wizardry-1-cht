#include "core/character.h"
#include "core/rules.h"
#include "wiz_test.h"

using namespace wiz::core;

// Mage at level 1 only memorises level-1 spells.
static void test_mage_lvl1_only_l1_slots() {
    Character c;
    c.klass = Klass::Mage;
    c.char_level = 1;
    recompute_spell_slots(c);
    WIZ_CHECK_EQ(int(c.mage_spell_slots[0]), 1);
    for (int i = 1; i < 7; ++i) WIZ_CHECK_EQ(int(c.mage_spell_slots[i]), 0);
    for (int i = 0; i < 7; ++i) WIZ_CHECK_EQ(int(c.priest_spell_slots[i]), 0);
}

// Mage at level 7 unlocks all spell levels (1..7), each with falling slots.
static void test_mage_lvl7_unlocks_all_levels() {
    Character c;
    c.klass = Klass::Mage;
    c.char_level = 7;
    recompute_spell_slots(c);
    WIZ_CHECK_EQ(int(c.mage_spell_slots[0]), 7);  // L1: 7
    WIZ_CHECK_EQ(int(c.mage_spell_slots[6]), 1);  // L7: 1
}

// Slots cap at 9 even at very high class levels.
static void test_slots_cap_at_nine() {
    Character c;
    c.klass = Klass::Mage;
    c.char_level = 20;
    recompute_spell_slots(c);
    for (int i = 0; i < 7; ++i) WIZ_CHECK_EQ(int(c.mage_spell_slots[i]), 9);
}

// Bishop gets mage from class lvl, priest from class lvl-3.
static void test_bishop_split_progression() {
    Character c;
    c.klass = Klass::Bishop;
    c.char_level = 5;
    recompute_spell_slots(c);
    WIZ_CHECK_EQ(int(c.mage_spell_slots[0]), 5);  // mage lvl1: 5
    WIZ_CHECK_EQ(int(c.priest_spell_slots[0]), 2);  // priest lvl1: 5-3=2
    WIZ_CHECK_EQ(int(c.priest_spell_slots[1]), 1);  // priest lvl2: 5-3-1=1
    WIZ_CHECK_EQ(int(c.priest_spell_slots[2]), 0);
}

// Fighter / Thief / Ninja never have slots.
static void test_non_caster_classes_get_nothing() {
    for (auto klass : {Klass::Fighter, Klass::Thief, Klass::Ninja}) {
        Character c;
        c.klass = klass;
        c.char_level = 13;
        recompute_spell_slots(c);
        for (int i = 0; i < 7; ++i) {
            WIZ_CHECK_EQ(int(c.mage_spell_slots[i]), 0);
            WIZ_CHECK_EQ(int(c.priest_spell_slots[i]), 0);
        }
    }
}

// consume_spell_slot returns true when no spellbook is loaded (test-friendly
// fallback) — wizcore tests don't load spells.json.
static void test_consume_slot_no_book_passes() {
    Character c;
    c.klass = Klass::Mage;
    c.char_level = 3;
    recompute_spell_slots(c);
    int before = int(c.mage_spell_slots[0]);
    // No spellbook → slot count must remain unchanged (consume returns true
    // but does not actually decrement).
    bool ok = consume_spell_slot(c, "HALITO");
    WIZ_CHECK(ok);
    WIZ_CHECK_EQ(int(c.mage_spell_slots[0]), before);
}

int main() {
    test_mage_lvl1_only_l1_slots();
    test_mage_lvl7_unlocks_all_levels();
    test_slots_cap_at_nine();
    test_bishop_split_progression();
    test_non_caster_classes_get_nothing();
    test_consume_slot_no_book_passes();
    return 0;
}
