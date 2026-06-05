#pragma once

#include <string>
#include <vector>

namespace wiz::data {

struct MonsterEntry {
    int id = 0;
    std::string name_en;
    std::string name_zh;
    std::string name_unknown;
    int ac = 10;
    int level = 1;
    long long exp = 0;
    std::string hp_dice;     // e.g. "2d6"
    std::string sprite_path; // relative path under assets/
};

const std::vector<MonsterEntry>& monsters();
const MonsterEntry* find_monster_by_id(int id);

struct ItemEntry {
    int id = 0;
    std::string name_en;
    std::string name_zh;
    std::string kind;
    long long price = 0;
    std::string dmg;
    int ac_mod = 0;
    int swings = 0;
    std::string use_classes;  // CSV of class letters (F/M/P/T/B/S/L/N)
    std::string spell;
};

// Loads items.json once; thereafter returns cached vector.
const std::vector<ItemEntry>& items();

// Lookup helpers
const ItemEntry* find_by_id(int id);

// Filter helpers
bool item_usable_by(const ItemEntry& it, char class_letter);

}  // namespace wiz::data
