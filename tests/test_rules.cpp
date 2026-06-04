#include "core/rules.h"
#include "wiz_test.h"

int main() {
    using namespace wiz::core;

    // XP curve sanity — higher levels need more XP
    WIZ_CHECK(xp_for_level(Klass::Fighter, 1) == 0);
    WIZ_CHECK(xp_for_level(Klass::Fighter, 2) > 0);
    WIZ_CHECK(xp_for_level(Klass::Fighter, 5) > xp_for_level(Klass::Fighter, 4));

    // Hit dice
    WIZ_CHECK_EQ(hit_die(Klass::Fighter), 8);
    WIZ_CHECK_EQ(hit_die(Klass::Mage), 4);
    WIZ_CHECK_EQ(hit_die(Klass::Lord), 10);

    // Class eligibility — snafaru v3.2: Ninja needs 15, not 17.
    Attributes good{15, 15, 15, 15, 15, 15};
    WIZ_CHECK(eligible_for_class(good, Alignment::Evil, Klass::Ninja));

    Attributes weak{14, 14, 14, 14, 14, 14};
    WIZ_CHECK(!eligible_for_class(weak, Alignment::Evil, Klass::Ninja));

    // Ninja must be evil
    WIZ_CHECK(!eligible_for_class(good, Alignment::Good, Klass::Ninja));

    // Recompute HP
    Character c;
    c.klass = Klass::Fighter;
    c.attr.vitality = 18;
    c.char_level = 5;
    recompute_derived(c);
    WIZ_CHECK(c.hp_max > 0);
    return 0;
}
