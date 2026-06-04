#include <array>

#include "core/combat.h"
#include "core/rng.h"
#include "wiz_test.h"

using namespace wiz::core;

static CombatGroup make_group(const char* nm, int count, int ac, int hp_n, int hp_d) {
    Monster m;
    m.name = nm;
    m.name_unknown = nm;
    m.armor_class = static_cast<std::int8_t>(ac);
    m.hp_dice_n = static_cast<std::uint8_t>(hp_n);
    m.hp_dice_d = static_cast<std::uint8_t>(hp_d);
    m.experience = 100;
    m.monster_class = 1;
    CombatGroup g;
    g.prototype = m;
    g.total_count = static_cast<std::int16_t>(count);
    g.alive_count = g.total_count;
    return g;
}

static Character make_caster() {
    Character c;
    c.name = "TESTER";
    c.char_level = 7;
    c.attr.strength = 12;
    c.attr.iq = 18;
    c.hp_left = 20;
    c.hp_max = 30;
    c.armor_class = 8;
    return c;
}

void test_halito_damage() {
    global_rng().reseed(42);
    CombatState s;
    begin_combat(s, {make_group("KOBOLD", 5, 7, 1, 4)});
    Character caster = make_caster();

    PlayerAction a;
    a.kind = PlayerAction::Spell;
    a.spell_name = "HALITO";
    a.target_group = 0;
    cast_spell(s, 0, caster, a);

    // Should have damaged the group
    WIZ_CHECK(s.groups[0].hp_total < 5 * 2);
    // Should have logged
    WIZ_CHECK(!s.log.empty());
}

void test_dios_self_heal() {
    global_rng().reseed(100);
    CombatState s;
    begin_combat(s, {make_group("KOBOLD", 1, 7, 1, 4)});
    Character caster = make_caster();
    caster.hp_left = 10;

    PlayerAction a;
    a.kind = PlayerAction::Spell;
    a.spell_name = "DIOS";
    cast_spell(s, 0, caster, a);

    WIZ_CHECK(caster.hp_left >= 11);
    WIZ_CHECK(caster.hp_left <= caster.hp_max);
}

void test_mogref_self_ac_buff() {
    global_rng().reseed(7);
    CombatState s;
    begin_combat(s, {make_group("KOBOLD", 1, 7, 1, 4)});
    Character caster = make_caster();

    PlayerAction a;
    a.kind = PlayerAction::Spell;
    a.spell_name = "MOGREF";
    cast_spell(s, 2, caster, a);

    WIZ_CHECK_EQ(int(s.per_member_ac[2]), -2);
}

void test_tiltowait_all_groups() {
    global_rng().reseed(99);
    CombatState s;
    begin_combat(s, {
        make_group("KOBOLD", 5, 7, 1, 4),
        make_group("ORC", 3, 6, 1, 6),
    });
    Character caster = make_caster();

    PlayerAction a;
    a.kind = PlayerAction::Spell;
    a.spell_name = "TILTOWAIT";
    cast_spell(s, 0, caster, a);

    // Both groups should be devastated (10d15 average 80 damage each)
    WIZ_CHECK(s.groups[0].alive_count == 0);
    WIZ_CHECK(s.groups[1].alive_count == 0);
}

void test_makanito_kills_low_level() {
    global_rng().reseed(50);
    CombatState s;
    auto group = make_group("WEAK", 10, 5, 1, 4);
    group.prototype.monster_class = 5;  // Below threshold
    begin_combat(s, {group});
    Character caster = make_caster();

    PlayerAction a;
    a.kind = PlayerAction::Spell;
    a.spell_name = "MAKANITO";
    a.target_group = 0;
    cast_spell(s, 0, caster, a);

    WIZ_CHECK_EQ(int(s.groups[0].alive_count), 0);
}

void test_makanito_fails_high_level() {
    global_rng().reseed(50);
    CombatState s;
    auto group = make_group("STRONG", 5, 0, 5, 8);
    group.prototype.monster_class = 12;  // Above threshold
    begin_combat(s, {group});
    Character caster = make_caster();

    PlayerAction a;
    a.kind = PlayerAction::Spell;
    a.spell_name = "MAKANITO";
    a.target_group = 0;
    cast_spell(s, 0, caster, a);

    WIZ_CHECK_EQ(int(s.groups[0].alive_count), 5);  // Unaffected
}

void test_unknown_spell_logged() {
    global_rng().reseed(1);
    CombatState s;
    begin_combat(s, {make_group("X", 1, 5, 1, 4)});
    Character caster = make_caster();
    std::size_t log_before = s.log.size();

    PlayerAction a;
    a.kind = PlayerAction::Spell;
    a.spell_name = "FAKESPELL";
    cast_spell(s, 0, caster, a);

    WIZ_CHECK(s.log.size() > log_before);
}

int main() {
    test_halito_damage();
    test_dios_self_heal();
    test_mogref_self_ac_buff();
    test_tiltowait_all_groups();
    test_makanito_kills_low_level();
    test_makanito_fails_high_level();
    test_unknown_spell_logged();
    return 0;
}
