#include "data/items_db.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

namespace wiz::data {

namespace {

std::vector<ItemEntry> g_items;
bool g_loaded = false;
std::vector<MonsterEntry> g_monsters;
bool g_monsters_loaded = false;

std::string str_field(const std::string& obj, const std::string& key) {
    auto pos = obj.find("\"" + key + "\"");
    if (pos == std::string::npos) return {};
    pos = obj.find(':', pos);
    if (pos == std::string::npos) return {};
    pos = obj.find('"', pos);
    if (pos == std::string::npos) return {};
    ++pos;
    std::string out;
    while (pos < obj.size() && obj[pos] != '"') {
        if (obj[pos] == '\\' && pos + 1 < obj.size()) {
            out.push_back(obj[pos + 1]);
            pos += 2;
        } else {
            out.push_back(obj[pos++]);
        }
    }
    return out;
}

long long int_field(const std::string& obj, const std::string& key, long long def = 0) {
    auto pos = obj.find("\"" + key + "\"");
    if (pos == std::string::npos) return def;
    pos = obj.find(':', pos);
    auto first = obj.find_first_not_of(" \t\n", pos + 1);
    auto end = obj.find_first_of(",}\n", first);
    try { return std::stoll(obj.substr(first, end - first)); }
    catch (...) { return def; }
}

std::string parse_use_array(const std::string& obj) {
    auto pos = obj.find("\"use\"");
    if (pos == std::string::npos) return {};
    pos = obj.find('[', pos);
    if (pos == std::string::npos) return {};
    auto end = obj.find(']', pos);
    std::string raw = obj.substr(pos + 1, end - pos - 1);
    std::string out;
    for (char c : raw) {
        if (c >= 'A' && c <= 'Z') out.push_back(c);
    }
    return out;
}

bool load_from_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        std::fprintf(stderr, "[items] cannot open %s\n", path.c_str());
        return false;
    }
    std::stringstream buf; buf << in.rdbuf();
    std::string text = buf.str();

    std::size_t pos = 0;
    while (pos < text.size()) {
        auto start = text.find('{', pos);
        if (start == std::string::npos) break;
        int depth = 1;
        auto end = start + 1;
        while (end < text.size() && depth > 0) {
            if (text[end] == '{') ++depth;
            else if (text[end] == '}') --depth;
            ++end;
        }
        std::string obj = text.substr(start, end - start);
        pos = end;

        ItemEntry e;
        e.id = static_cast<int>(int_field(obj, "id"));
        e.name_en = str_field(obj, "name");
        e.name_zh = str_field(obj, "name_zh");
        e.kind = str_field(obj, "kind");
        e.price = int_field(obj, "price");
        e.dmg = str_field(obj, "dmg");
        e.ac_mod = static_cast<int>(int_field(obj, "ac_mod"));
        e.swings = static_cast<int>(int_field(obj, "swings"));
        e.spell = str_field(obj, "spell");
        e.use_classes = parse_use_array(obj);
        if (!e.name_en.empty()) g_items.push_back(std::move(e));
    }
    return !g_items.empty();
}

}  // namespace

const std::vector<ItemEntry>& items() {
    if (!g_loaded) {
        g_loaded = true;
        // Try multiple plausible asset paths
#ifdef WIZ_ASSETS_DIR
        load_from_file(std::string(WIZ_ASSETS_DIR) + "/data/items.json");
#else
        load_from_file("assets/data/items.json");
#endif
        std::fprintf(stderr, "[items] loaded %zu items\n", g_items.size());
    }
    return g_items;
}

const ItemEntry* find_by_id(int id) {
    for (const auto& e : items()) if (e.id == id) return &e;
    return nullptr;
}

bool item_usable_by(const ItemEntry& it, char class_letter) {
    return it.use_classes.find(class_letter) != std::string::npos;
}

namespace {
bool load_monsters_from_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        std::fprintf(stderr, "[monsters] cannot open %s\n", path.c_str());
        return false;
    }
    std::stringstream buf; buf << in.rdbuf();
    std::string text = buf.str();

    std::size_t pos = 0;
    while (pos < text.size()) {
        auto start = text.find('{', pos);
        if (start == std::string::npos) break;
        int depth = 1;
        auto end = start + 1;
        while (end < text.size() && depth > 0) {
            if (text[end] == '{') ++depth;
            else if (text[end] == '}') --depth;
            ++end;
        }
        std::string obj = text.substr(start, end - start);
        pos = end;

        MonsterEntry m;
        m.id = static_cast<int>(int_field(obj, "id"));
        m.name_en = str_field(obj, "name");
        m.name_zh = str_field(obj, "name_zh");
        m.name_unknown = str_field(obj, "name_unk");
        m.ac = static_cast<int>(int_field(obj, "ac"));
        m.level = static_cast<int>(int_field(obj, "lvl", 1));
        m.exp = int_field(obj, "exp");
        m.hp_dice = str_field(obj, "hp");
        m.sprite_path = str_field(obj, "sprite");
        if (!m.name_en.empty()) g_monsters.push_back(std::move(m));
    }
    return !g_monsters.empty();
}
}  // namespace

const std::vector<MonsterEntry>& monsters() {
    if (!g_monsters_loaded) {
        g_monsters_loaded = true;
#ifdef WIZ_ASSETS_DIR
        load_monsters_from_file(std::string(WIZ_ASSETS_DIR) + "/data/monsters.json");
#else
        load_monsters_from_file("assets/data/monsters.json");
#endif
        std::fprintf(stderr, "[monsters] loaded %zu monsters\n", g_monsters.size());
    }
    return g_monsters;
}

const MonsterEntry* find_monster_by_id(int id) {
    for (const auto& m : monsters()) if (m.id == id) return &m;
    return nullptr;
}

}  // namespace wiz::data
