#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "core/arith.h"

namespace wiz::core {

enum class Race : std::uint8_t { Human = 0, Elf, Dwarf, Gnome, Hobbit, Count };
enum class Klass : std::uint8_t {
    Fighter = 0, Mage, Priest, Thief, Bishop, Samurai, Lord, Ninja, Count
};
enum class Alignment : std::uint8_t { Good = 0, Neutral, Evil, Count };
enum class Status : std::uint8_t {
    Ok = 0, Afraid, Asleep, Paralyzed, Stoned, Dead, Ashes, Lost, Count
};

struct Attributes {
    std::uint8_t strength = 0;
    std::uint8_t iq = 0;
    std::uint8_t piety = 0;
    std::uint8_t vitality = 0;
    std::uint8_t agility = 0;
    std::uint8_t luck = 0;
};

struct ItemSlot {
    std::int16_t item_id = -1;
    bool equipped = false;
    bool identified = false;
    bool cursed = false;
};

// Mirrors TCHAR from WIZ.TEXT lines 62-105.
// Layout intentionally does NOT match the Pascal PACKED format byte-for-byte;
// we'll handle Apple II save imports with an explicit deserializer if needed.
struct Character {
    std::string name;
    std::string password;
    bool in_maze = false;
    Race race = Race::Human;
    Klass klass = Klass::Fighter;
    Alignment alignment = Alignment::Neutral;
    Status status = Status::Ok;
    std::uint16_t age = 14 * 52;  // weeks, original game uses weeks/year
    Attributes attr{};
    long_t gold = 0;
    long_t experience = 0;
    std::uint16_t max_level_ac = 0;
    std::uint8_t char_level = 1;
    std::int16_t hp_left = 0;
    std::int16_t hp_max = 0;
    std::uint8_t armor_class = 10;

    std::array<bool, 51> spells_known{};
    std::array<std::uint8_t, 7> mage_spell_slots{};
    std::array<std::uint8_t, 7> priest_spell_slots{};

    std::array<ItemSlot, 8> inventory{};
};

const char* race_name(Race r);
const char* klass_name(Klass k);
const char* alignment_name(Alignment a);
const char* status_name(Status s);

}  // namespace wiz::core
