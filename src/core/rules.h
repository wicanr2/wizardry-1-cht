#pragma once

#include "core/character.h"

namespace wiz::core {

// Game mechanics — derived from Wizardry I source (WIZ.TEXT, UTILITIE.TEXT,
// COMBAT.TEXT). All snafaru v3.2 fixes are absorbed here.

// XP needed to reach the given level for the given class.
// Original tables are stored on disk; this is the standard W1 progression
// approximated for testing. Replace with exact tables after M4 disk-extract.
long_t xp_for_level(Klass k, std::uint8_t level) noexcept;

// Hit dice per class — d8/d6/d10 depending on klass.
std::uint8_t hit_die(Klass k) noexcept;

// Recompute derived stats (max HP, AC base) from current attributes and level.
void recompute_derived(Character& c) noexcept;

// Class eligibility — Ninja requires 15+ in all (snafaru v3.2 change from 17).
bool eligible_for_class(const Attributes& a, Alignment align, Klass want) noexcept;

// Roll initial attributes for race; returns base values before bonus points.
Attributes base_attributes_for(Race r) noexcept;

// Bonus points for character roller — typical 5..10, occasionally 20+.
int roll_bonus_points() noexcept;

}  // namespace wiz::core
