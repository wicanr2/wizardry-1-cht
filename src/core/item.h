#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "core/arith.h"

namespace wiz::core {

enum class ItemKind : std::uint8_t {
    Weapon = 0, Armor, Shield, Helmet, Gauntlets, Misc, Scroll, Potion, Count
};

// Mirrors TOBJREC from WIZ.TEXT lines 107-132.
struct Item {
    std::string name;
    std::string name_unknown;
    ItemKind kind = ItemKind::Misc;
    std::uint8_t alignment_mask = 0;   // which alignments can use it
    bool cursed = false;
    std::int16_t special = 0;          // special effect id
    std::int16_t change_to = -1;       // morphs into this id when broken
    std::uint8_t change_chance = 0;
    long_t price = 0;
    std::int16_t boltac_xx = 0;        // boltac flag
    std::int16_t spell_power = -1;     // cast spell when used
    std::array<bool, 8> class_use{};   // indexed by Klass
    std::int16_t heal_pts = 0;
    std::int16_t armor_mod = 0;
    std::int16_t weapon_hit_mod = 0;
    std::int16_t weapon_dmg_n = 0;
    std::int16_t weapon_dmg_d = 0;
    std::int16_t weapon_dmg_plus = 0;
    std::uint8_t extra_swings = 0;
    std::int16_t crit_hit_mod = 0;
};

}  // namespace wiz::core
