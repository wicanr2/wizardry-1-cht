// v1.14: Class-change requirement enforcement + apply effects.
#include "core/character.h"
#include "game/class_change.h"
#include "wiz_test.h"

using namespace wiz::core;
using wiz::game::can_change_to;
using wiz::game::apply_class_change;

static Character make_fighter() {
    Character c;
    c.name = "TEST";
    c.klass = Klass::Fighter;
    c.alignment = Alignment::Good;
    c.attr = {18, 18, 18, 18, 18, 18};
    c.char_level = 10;
    c.experience = 50000;
    c.hp_max = 60;
    c.hp_left = 60;
    return c;
}

static void test_same_class_rejected() {
    auto c = make_fighter();
    WIZ_CHECK(!can_change_to(c, Klass::Fighter).empty());
}

static void test_low_stats_rejected() {
    auto c = make_fighter();
    c.attr = {8, 8, 8, 8, 8, 8};  // below every minimum
    WIZ_CHECK(!can_change_to(c, Klass::Lord).empty());
}

static void test_good_alignment_can_lord() {
    auto c = make_fighter();
    WIZ_CHECK(can_change_to(c, Klass::Lord).empty());
}

static void test_neutral_alignment_blocked_lord() {
    auto c = make_fighter();
    c.alignment = Alignment::Neutral;
    WIZ_CHECK(!can_change_to(c, Klass::Lord).empty());
}

static void test_evil_alignment_blocked_lord() {
    auto c = make_fighter();
    c.alignment = Alignment::Evil;
    WIZ_CHECK(!can_change_to(c, Klass::Lord).empty());
}

static void test_evil_can_ninja() {
    auto c = make_fighter();
    c.alignment = Alignment::Evil;
    WIZ_CHECK(can_change_to(c, Klass::Ninja).empty());
}

static void test_apply_resets_level_and_halves_hp() {
    auto c = make_fighter();
    int hp_before = c.hp_max;
    int age_before = c.age;
    apply_class_change(c, Klass::Lord);
    WIZ_CHECK_EQ(int(c.klass), int(Klass::Lord));
    WIZ_CHECK_EQ(int(c.char_level), 1);
    WIZ_CHECK_EQ(int(c.experience), 0);
    WIZ_CHECK_EQ(int(c.hp_max), hp_before / 2);
    WIZ_CHECK(c.age > age_before);
}

int main() {
    test_same_class_rejected();
    test_low_stats_rejected();
    test_good_alignment_can_lord();
    test_neutral_alignment_blocked_lord();
    test_evil_alignment_blocked_lord();
    test_evil_can_ninja();
    test_apply_resets_level_and_halves_hp();
    return 0;
}
