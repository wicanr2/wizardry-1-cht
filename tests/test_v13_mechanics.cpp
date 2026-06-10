// v1.13: Poison tick + sleep/paralyze gating + back-row melee restriction.
#include <array>

#include "core/combat.h"
#include "core/rng.h"
#include "wiz_test.h"

using namespace wiz::core;

static Character make_char(const char* name, int hp = 30, int slot_ac = 4) {
    Character c;
    c.name = name;
    c.char_level = 5;
    c.attr.strength = 18;
    c.hp_max = hp;
    c.hp_left = hp;
    c.armor_class = slot_ac;
    return c;
}

static CombatGroup make_group(const char* name, int count = 3) {
    Monster m;
    m.name = name;
    m.name_unknown = name;
    m.armor_class = 10;
    m.hp_dice_n = 1;
    m.hp_dice_d = 4;
    CombatGroup g;
    g.prototype = m;
    g.total_count = count;
    g.alive_count = count;
    return g;
}

static void test_poison_tick_drains_hp() {
    global_rng().reseed(11);
    std::array<Character, 6> party{};
    party[0] = make_char("FRONT");
    party[0].status = Status::Poisoned;
    party[0].poison_strength = 3;
    int hp_before = party[0].hp_left;

    CombatState s;
    begin_combat(s, {make_group("KOBOLD")});
    PlayerAction p;
    p.kind = PlayerAction::Parry;
    set_action(s, 0, p);
    for (int i = 1; i < 6; ++i) set_action(s, i, p);
    resolve_round(s, party);

    WIZ_CHECK(party[0].hp_left < hp_before);
}

static void test_poison_lethal_transitions_to_dead() {
    global_rng().reseed(22);
    std::array<Character, 6> party{};
    party[0] = make_char("WEAK", /*hp*/ 2);
    party[0].status = Status::Poisoned;
    party[0].poison_strength = 5;

    CombatState s;
    begin_combat(s, {make_group("KOBOLD")});
    PlayerAction p;
    p.kind = PlayerAction::Parry;
    for (int i = 0; i < 6; ++i) set_action(s, i, p);
    resolve_round(s, party);

    WIZ_CHECK_EQ(int(party[0].status), int(Status::Dead));
    WIZ_CHECK_EQ(int(party[0].poison_strength), 0);
}

static void test_back_row_melee_fails() {
    global_rng().reseed(33);
    std::array<Character, 6> party{};
    for (int i = 0; i < 6; ++i) party[i] = make_char("HERO");
    CombatState s;
    begin_combat(s, {make_group("KOBOLD")});
    PlayerAction fight;
    fight.kind = PlayerAction::Fight;
    fight.target_group = 0;
    PlayerAction parry;
    parry.kind = PlayerAction::Parry;
    set_action(s, 5, fight);          // back row
    for (int i = 0; i < 5; ++i) set_action(s, i, parry);

    int hp_before = s.groups[0].hp_total;
    resolve_round(s, party);
    // Back row swing must not have reduced enemy HP.
    WIZ_CHECK_EQ(s.groups[0].hp_total, hp_before);
}

static void test_sleeping_group_skips_action() {
    global_rng().reseed(44);
    std::array<Character, 6> party{};
    party[0] = make_char("BAIT", /*hp*/ 100);
    CombatState s;
    begin_combat(s, {make_group("BIG_BAT", 5)});
    s.groups[0].asleep = true;

    PlayerAction parry;
    parry.kind = PlayerAction::Parry;
    for (int i = 0; i < 6; ++i) set_action(s, i, parry);
    int hp_before = party[0].hp_left;
    resolve_round(s, party);

    WIZ_CHECK_EQ(int(party[0].hp_left), hp_before);  // sleeping = no swing
}

int main() {
    test_poison_tick_drains_hp();
    test_poison_lethal_transitions_to_dead();
    test_back_row_melee_fails();
    test_sleeping_group_skips_action();
    return 0;
}
