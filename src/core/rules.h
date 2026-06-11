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

// Recompute spell-slot caps and auto-learn castable spells for the given
// character. mage_spell_slots[L-1] and priest_spell_slots[L-1] become the
// per-rest maximum (call this on level-up and on resting at the Inn).
// Per-school access:
//   Mage    — mage slots from class_level
//   Priest  — priest slots from class_level
//   Bishop  — mage from class_level, priest from class_level-3
//   Samurai — mage from class_level-3
//   Lord    — priest from class_level-3
//   Fighter / Thief / Ninja — none
void recompute_spell_slots(Character& c) noexcept;

// Restore spell slots to their per-rest cap (no level-up). Idempotent.
void restore_spell_slots(Character& c) noexcept;

// Decrement one slot of the appropriate school/level for the given spell.
// Returns false if the caster has 0 slots remaining (or the spell name is
// not in the catalogue). Used by combat.cpp and camp.cpp before resolving
// the effect — caller should bail and log a "no charges left" message.
bool consume_spell_slot(Character& c, std::string_view spell_name);

}  // namespace wiz::core
