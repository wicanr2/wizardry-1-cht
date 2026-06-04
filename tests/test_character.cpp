#include <string>

#include "core/character.h"
#include "wiz_test.h"

int main() {
    using namespace wiz::core;
    Character c;
    c.name = "Werdna";
    c.race = Race::Elf;
    c.klass = Klass::Mage;
    c.alignment = Alignment::Evil;
    c.char_level = 13;
    c.hp_max = 88;
    c.hp_left = 88;

    WIZ_CHECK_EQ(std::string(race_name(c.race)), std::string("elf"));
    WIZ_CHECK_EQ(std::string(klass_name(c.klass)), std::string("mage"));
    WIZ_CHECK_EQ(std::string(alignment_name(c.alignment)), std::string("evil"));
    WIZ_CHECK_EQ(c.hp_left, 88);
    return 0;
}
