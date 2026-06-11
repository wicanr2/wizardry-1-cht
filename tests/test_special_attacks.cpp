#include <array>

#include "core/character.h"
#include "core/combat.h"
#include "core/rng.h"
#include "wiz_test.h"

using namespace wiz::core;

static CombatGroup make_group(const char* name, int ac, int hp_n, int hp_d, int count) {
    Monster m;
    m.name = name;
    m.name_unknown = name;
    m.armor_class = ac;
    m.hp_dice_n = hp_n;
    m.hp_dice_d = hp_d;
    CombatGroup g;
    g.prototype = m;
    g.total_count = count;
    g.alive_count = count;
    return g;
}

static Character make_lvl5() {
    Character c;
    c.name = "TESTER";
    c.char_level = 5;
    c.hp_max = 40;
    c.hp_left = 40;
    c.armor_class = 4;
    c.attr.strength = 16;
    c.attr.vitality = 14;
    c.status = Status::Ok;
    return c;
}

// 1. Surprise — across many seeds, both surprise states should fire at least
//    once and the round counter advances each resolve_round.
static void test_surprise_fires_and_round_advances() {
    int party_ahead = 0, monsters_ahead = 0;
    for (int seed = 0; seed < 200; ++seed) {
        global_rng().reseed(static_cast<std::uint32_t>(seed));
        CombatState s;
        begin_combat(s, {make_group("KOBOLD", 7, 1, 4, 2)});
        if (s.surprise == Surprise::PartyAhead) ++party_ahead;
        else if (s.surprise == Surprise::MonstersAhead) ++monsters_ahead;
    }
    WIZ_CHECK(party_ahead > 10);
    WIZ_CHECK(monsters_ahead > 10);

    // Round counter starts at 0, increments after each resolve.
    global_rng().reseed(99);
    CombatState s;
    begin_combat(s, {make_group("KOBOLD", 7, 1, 4, 1)});
    std::array<Character, 6> party{};
    party[0] = make_lvl5();
    WIZ_CHECK_EQ(s.round, 0);
    PlayerAction p; p.kind = PlayerAction::Parry; set_action(s, 0, p);
    resolve_round(s, party);
    WIZ_CHECK_EQ(s.round, 1);
}

// 2. Vampire drain — across rounds with a hit, char_level should drop.
//    Seeded so the vampire reliably lands at least one hit in 20 rounds.
static void test_vampire_drains_level() {
    bool drained_at_least_once = false;
    for (int seed = 100; seed < 130 && !drained_at_least_once; ++seed) {
        global_rng().reseed(static_cast<std::uint32_t>(seed));
        CombatState s;
        begin_combat(s, {make_group("VAMPIRE", 0, 10, 8, 1)});
        std::array<Character, 6> party{};
        party[0] = make_lvl5();
        party[0].hp_max = 200;
        party[0].hp_left = 200;  // can't die during the test
        PlayerAction parry; parry.kind = PlayerAction::Parry;
        set_action(s, 0, parry);
        int start_lvl = party[0].char_level;
        for (int r = 0; r < 20 && party[0].status != Status::Dead; ++r) {
            resolve_round(s, party);
            if (party[0].char_level < start_lvl) {
                drained_at_least_once = true;
                break;
            }
        }
    }
    WIZ_CHECK(drained_at_least_once);
}

// 3. Ghoul paralysis — across rounds with a hit, party member should
//    eventually end up Paralyzed.
static void test_ghoul_can_paralyse() {
    bool paralysed = false;
    for (int seed = 200; seed < 240 && !paralysed; ++seed) {
        global_rng().reseed(static_cast<std::uint32_t>(seed));
        CombatState s;
        begin_combat(s, {make_group("GHOUL", 6, 2, 8, 2)});
        std::array<Character, 6> party{};
        party[0] = make_lvl5();
        party[0].hp_max = 999;
        party[0].hp_left = 999;
        PlayerAction parry; parry.kind = PlayerAction::Parry;
        set_action(s, 0, parry);
        for (int r = 0; r < 20; ++r) {
            resolve_round(s, party);
            if (party[0].status == Status::Paralyzed) {
                paralysed = true;
                break;
            }
        }
    }
    WIZ_CHECK(paralysed);
}

// 4. Dragon breath — at least one round should log "吐息" and damage the
//    whole party (everyone takes hp loss in one tick).
static void test_dragon_breathes_at_all_party() {
    bool everyone_hit = false;
    for (int seed = 300; seed < 320 && !everyone_hit; ++seed) {
        global_rng().reseed(static_cast<std::uint32_t>(seed));
        CombatState s;
        begin_combat(s, {make_group("GAS DRAGON", 4, 6, 8, 1)});
        std::array<Character, 6> party{};
        for (int i = 0; i < 6; ++i) {
            party[i] = make_lvl5();
            party[i].name = "M" + std::to_string(i);
            party[i].hp_max = 200;
            party[i].hp_left = 200;
        }
        PlayerAction parry; parry.kind = PlayerAction::Parry;
        for (int i = 0; i < 6; ++i) set_action(s, i, parry);

        int before[6];
        for (int i = 0; i < 6; ++i) before[i] = party[i].hp_left;
        resolve_round(s, party);
        bool all = true;
        for (int i = 0; i < 6; ++i) {
            if (party[i].hp_left >= before[i]) { all = false; break; }
        }
        if (all) everyone_hit = true;
    }
    WIZ_CHECK(everyone_hit);
}

int main() {
    test_surprise_fires_and_round_advances();
    test_vampire_drains_level();
    test_ghoul_can_paralyse();
    test_dragon_breathes_at_all_party();
    return 0;
}
