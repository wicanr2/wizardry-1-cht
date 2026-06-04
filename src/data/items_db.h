#pragma once

#include <string>
#include <vector>

namespace wiz::data {

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
