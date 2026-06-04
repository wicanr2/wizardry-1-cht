#include <array>

#include "core/combat.h"
#include "core/rng.h"
#include "wiz_test.h"

int main() {
    using namespace wiz::core;
    global_rng().reseed(12345);

    CombatState s;
    Monster kobold;
    kobold.name = "KOBOLD";
    kobold.name_unknown = "DOG-LIKE";
    kobold.armor_class = 7;
    kobold.hp_dice_n = 1;
    kobold.hp_dice_d = 4;
    kobold.experience = 415;

    CombatGroup g;
    g.prototype = kobold;
    g.total_count = 3;
    g.alive_count = 3;

    begin_combat(s, {g});
    WIZ_CHECK_EQ(int(s.phase), int(CombatPhase::PartyAction));
    WIZ_CHECK_EQ(int(s.outcome), int(CombatOutcome::Ongoing));
    WIZ_CHECK_EQ(s.groups.size(), 1u);

    std::array<Character, 6> party{};
    party[0].name = "TEST";
    party[0].char_level = 5;
    party[0].attr.strength = 18;
    party[0].hp_max = 30;
    party[0].hp_left = 30;
    party[0].armor_class = 4;

    PlayerAction fight;
    fight.kind = PlayerAction::Fight;
    fight.target_group = 0;
    set_action(s, 0, fight);

    // Run a few rounds with reset RNG; just ensure it terminates / no crash.
    for (int i = 0; i < 50 && s.outcome == CombatOutcome::Ongoing; ++i) {
        resolve_round(s, party);
    }
    WIZ_CHECK(s.outcome != CombatOutcome::Ongoing);
    return 0;
}
