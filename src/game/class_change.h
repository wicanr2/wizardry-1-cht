#pragma once

#include <string>
#include <string_view>

#include "core/character.h"

namespace wiz::game {

// Per-class minimum attribute requirements (snafaru v3.2 reconciled).
struct ClassRequirement {
    std::uint8_t str = 0;
    std::uint8_t iq  = 0;
    std::uint8_t pie = 0;
    std::uint8_t vit = 0;
    std::uint8_t agi = 0;
    std::uint8_t luc = 0;
};

ClassRequirement requirements_for(core::Klass k);

// Returns "" if the character can switch into `target`, else a short
// reason ("屬性不足", "陣營禁止", "與目前職業相同").
std::string can_change_to(const core::Character& c, core::Klass target);

// Apply the change: level → 1, halve hp_max, clear spell slots (but keep
// spells_known), age + 50 weeks. Caller must have already verified with
// can_change_to(). Returns the new class name for the log.
std::string_view apply_class_change(core::Character& c, core::Klass target);

}  // namespace wiz::game
