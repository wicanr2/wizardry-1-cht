#pragma once

#include <array>
#include <cstdint>
#include <string>

namespace wiz::core {

enum class SpellSchool : std::uint8_t { Mage = 0, Priest = 1 };
enum class SpellTarget : std::uint8_t { Self, Ally, Party, Enemy, EnemyGroup, AllEnemies };

struct Spell {
    std::string name;       // e.g. "KATINO"
    std::string description;
    SpellSchool school = SpellSchool::Mage;
    std::uint8_t level = 1;     // 1..7
    SpellTarget target = SpellTarget::Self;
    std::uint8_t group = 0;     // matches SPELLGRP in TSCNTOC
    std::uint8_t type = 0;      // matches SPELL012 in TSCNTOC (0/1/2 effect class)
    bool combat_only = false;
    bool camp_only = false;
};

// The fixed spell catalogue (51 entries to match SPELLHSH[51]).
// Loaded at startup from assets/data/spells.json.
class SpellBook {
   public:
    bool load(const std::string& path);
    [[nodiscard]] const Spell* find(std::string_view name) const noexcept;
    [[nodiscard]] const Spell* at(std::size_t idx) const noexcept;
    [[nodiscard]] std::size_t size() const noexcept { return spells_.size(); }

   private:
    std::array<Spell, 51> spells_{};
    std::size_t count_ = 0;
};

SpellBook& global_spellbook() noexcept;

}  // namespace wiz::core
