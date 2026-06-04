#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "core/arith.h"

namespace wiz::core {

// Mirrors TENEMY from WIZ.TEXT lines 164-190.
struct Monster {
    std::string name_unknown;
    std::string name_unknown_plural;
    std::string name;
    std::string name_plural;

    std::uint8_t picture_id = 0;
    long_t calc1 = 0;
    std::uint8_t hp_dice_n = 0;
    std::uint8_t hp_dice_d = 0;
    std::uint8_t monster_class = 0;
    std::int8_t armor_class = 10;
    std::uint8_t reward_special_count = 0;
    std::array<std::uint8_t, 7> reward_specials{};
    long_t experience = 0;
    std::uint8_t drain_amount = 0;
    std::int16_t heal_pts = 0;
    std::uint8_t reward1 = 0;
    std::uint8_t reward2 = 0;
    std::uint8_t enemy_team = 0;
    std::uint8_t team_percent = 0;
    std::uint8_t mage_spells_max = 0;
    std::uint8_t priest_spells_max = 0;
    std::uint8_t unique_id = 0;
    std::uint8_t breath_mask = 0;
    std::uint8_t unaffected_mask = 0;
    std::array<std::uint8_t, 16> weapon_vs_type3{};
    std::array<std::uint8_t, 16> special_attacks{};
};

}  // namespace wiz::core
