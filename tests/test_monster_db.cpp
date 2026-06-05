#include "data/items_db.h"
#include "wiz_test.h"

#include <string>

int main() {
    using namespace wiz::data;
    const auto& ms = monsters();
    WIZ_CHECK(ms.size() >= 30);

    // Werdna (id 25) should have a sprite
    auto* werdna = find_monster_by_id(25);
    WIZ_CHECK(werdna != nullptr);
    WIZ_CHECK_EQ(werdna->name_en, std::string("WERDNA"));
    WIZ_CHECK_EQ(werdna->name_zh, std::string("沃登納（瘋王）"));
    WIZ_CHECK(!werdna->sprite_path.empty());
    WIZ_CHECK(werdna->sprite_path.find("Werdna") != std::string::npos);

    // Kobold (id 2)
    auto* kobold = find_monster_by_id(2);
    WIZ_CHECK(kobold != nullptr);
    WIZ_CHECK_EQ(kobold->name_zh, std::string("狗頭人"));
    WIZ_CHECK(kobold->sprite_path.find("Kobold") != std::string::npos);

    // Count how many monsters got mapped to sprites
    int with_sprite = 0;
    for (const auto& m : ms) {
        if (!m.sprite_path.empty()) ++with_sprite;
    }
    WIZ_CHECK(with_sprite >= 29);
    return 0;
}
