#include "data/maze_db.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace wiz::data {

namespace {

using core::MazeLevel;
using core::Wall;
using core::SquareFeature;

// Reserved square-type slot assignments. Slot 0 is implicit None.
constexpr std::uint8_t kSlotStairs     = 1;
constexpr std::uint8_t kSlotChute      = 2;
constexpr std::uint8_t kSlotSpinner    = 3;
constexpr std::uint8_t kSlotPit        = 4;
constexpr std::uint8_t kSlotTeleporter = 5;
constexpr std::uint8_t kSlotElevator   = 6;
constexpr std::uint8_t kSlotMessage    = 7;
constexpr std::uint8_t kSlotEncounter  = 8;

std::vector<MazeLevel> g_mazes;
bool g_loaded = false;

// Tiny JSON helpers tailored to the wiz1_mazes.json shape.
std::string str_field(std::string_view obj, std::string_view key) {
    std::string needle = "\"" + std::string(key) + "\"";
    auto pos = obj.find(needle);
    if (pos == std::string_view::npos) return {};
    pos = obj.find(':', pos);
    if (pos == std::string_view::npos) return {};
    pos = obj.find('"', pos);
    if (pos == std::string_view::npos) return {};
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

long long int_field(std::string_view obj, std::string_view key, long long def = 0) {
    std::string needle = "\"" + std::string(key) + "\"";
    auto pos = obj.find(needle);
    if (pos == std::string_view::npos) return def;
    pos = obj.find(':', pos);
    if (pos == std::string_view::npos) return def;
    auto first = obj.find_first_not_of(" \t\n", pos + 1);
    auto end = obj.find_first_of(",}\n]", first);
    try {
        return std::stoll(std::string{obj.substr(first, end - first)});
    } catch (...) { return def; }
}

bool bool_field(std::string_view obj, std::string_view key) {
    std::string needle = "\"" + std::string(key) + "\"";
    auto pos = obj.find(needle);
    if (pos == std::string_view::npos) return false;
    pos = obj.find(':', pos);
    auto t = obj.find("true", pos);
    auto f = obj.find("false", pos);
    auto stop = obj.find_first_of(",}\n", pos + 1);
    if (t != std::string_view::npos && t < stop) return true;
    (void)f;
    return false;
}

Wall wall_from_string(const std::string& s) {
    if (s == "wall")        return Wall::Wall;
    if (s == "door")        return Wall::Door;
    if (s == "hidden_door") return Wall::HiddenDoor;
    if (s == "one_way")     return Wall::Door;  // approximate
    return Wall::Open;
}

std::uint8_t slot_for_feature(const std::string& f) {
    if (f == "stairs_up" || f == "stairs_down") return kSlotStairs;
    if (f == "chute")                            return kSlotChute;
    if (f == "spinner")                          return kSlotSpinner;
    if (f == "pit")                              return kSlotPit;
    if (f == "teleporter")                       return kSlotTeleporter;
    if (f == "elevator")                         return kSlotElevator;
    if (f == "key_item" || f == "marker")        return kSlotMessage;
    return 0;
}

void set_default_outer_walls(MazeLevel& m) {
    for (int y = 0; y < MazeLevel::kSize; ++y) {
        for (int x = 0; x < MazeLevel::kSize; ++x) {
            m.west[y][x]  = (x == 0)                          ? Wall::Wall : Wall::Open;
            m.east[y][x]  = (x == MazeLevel::kSize - 1)       ? Wall::Wall : Wall::Open;
            m.north[y][x] = (y == 0)                          ? Wall::Wall : Wall::Open;
            m.south[y][x] = (y == MazeLevel::kSize - 1)       ? Wall::Wall : Wall::Open;
        }
    }
}

void register_square_types(MazeLevel& m) {
    m.sqre_type[kSlotStairs]     = SquareFeature::Stairs;
    m.sqre_type[kSlotChute]      = SquareFeature::Chute;
    m.sqre_type[kSlotSpinner]    = SquareFeature::Spinner;
    m.sqre_type[kSlotPit]        = SquareFeature::Pit;
    m.sqre_type[kSlotTeleporter] = SquareFeature::Teleporter;
    m.sqre_type[kSlotElevator]   = SquareFeature::Elevator;
    m.sqre_type[kSlotMessage]    = SquareFeature::Message;
    m.sqre_type[kSlotEncounter]  = SquareFeature::Encounter;
}

// Slice text[pos..] up to the matching close-bracket of a nested object/array.
// Assumes pos is on the opening bracket char (`{` or `[`).
std::size_t match_bracket(const std::string& text, std::size_t pos) {
    char open = text[pos], close = (open == '{') ? '}' : ']';
    int depth = 1;
    ++pos;
    while (pos < text.size() && depth > 0) {
        char c = text[pos];
        if (c == '"') {
            ++pos;
            while (pos < text.size() && text[pos] != '"') {
                if (text[pos] == '\\' && pos + 1 < text.size()) pos += 2;
                else ++pos;
            }
            if (pos < text.size()) ++pos;
            continue;
        }
        if (c == open)  ++depth;
        if (c == close) --depth;
        ++pos;
    }
    return pos;
}

void parse_cell(const std::string& cell, MazeLevel& m) {
    int x = static_cast<int>(int_field(cell, "x", -1));
    int y = static_cast<int>(int_field(cell, "y", -1));
    if (x < 0 || y < 0 || x >= MazeLevel::kSize || y >= MazeLevel::kSize) return;

    std::string n = str_field(cell, "n");
    std::string e = str_field(cell, "e");
    std::string s = str_field(cell, "s");
    std::string w = str_field(cell, "w");
    if (!n.empty()) {
        m.north[y][x] = wall_from_string(n);
        if (y > 0) m.south[y - 1][x] = m.north[y][x];
    }
    if (!s.empty()) {
        m.south[y][x] = wall_from_string(s);
        if (y < MazeLevel::kSize - 1) m.north[y + 1][x] = m.south[y][x];
    }
    if (!e.empty()) {
        m.east[y][x] = wall_from_string(e);
        if (x < MazeLevel::kSize - 1) m.west[y][x + 1] = m.east[y][x];
    }
    if (!w.empty()) {
        m.west[y][x] = wall_from_string(w);
        if (x > 0) m.east[y][x - 1] = m.west[y][x];
    }

    std::string feature = str_field(cell, "feature");
    if (!feature.empty()) {
        std::uint8_t slot = slot_for_feature(feature);
        if (slot) m.sqr_extra[y][x] = slot;
    }
    if (bool_field(cell, "encounter")) {
        m.fights[y][x] = 1;
    }
    // Dark zones not yet rendered, but recorded if we add lighting later.
}

void parse_floor(const std::string& floor_obj, MazeLevel& m) {
    m = MazeLevel{};
    int level = static_cast<int>(int_field(floor_obj, "level", 1));
    m.level_number = static_cast<std::uint8_t>(level);
    set_default_outer_walls(m);
    register_square_types(m);

    // Find the "cells" array.
    auto cells_pos = floor_obj.find("\"cells\"");
    if (cells_pos == std::string::npos) return;
    auto array_pos = floor_obj.find('[', cells_pos);
    if (array_pos == std::string::npos) return;
    auto array_end = match_bracket(floor_obj, array_pos);

    // Iterate cell objects.
    std::size_t cur = array_pos + 1;
    while (cur < array_end) {
        auto obj_start = floor_obj.find('{', cur);
        if (obj_start == std::string::npos || obj_start >= array_end) break;
        auto obj_end = match_bracket(floor_obj, obj_start);
        if (obj_end > array_end) break;
        parse_cell(floor_obj.substr(obj_start, obj_end - obj_start), m);
        cur = obj_end;
    }
}

bool load_from_file(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        std::fprintf(stderr, "[mazes] cannot open %s\n", path.c_str());
        return false;
    }
    std::stringstream buf;
    buf << in.rdbuf();
    std::string text = buf.str();

    // Find the "floors" array.
    auto floors_pos = text.find("\"floors\"");
    if (floors_pos == std::string::npos) return false;
    auto array_pos = text.find('[', floors_pos);
    if (array_pos == std::string::npos) return false;
    auto array_end = match_bracket(text, array_pos);

    g_mazes.clear();
    std::size_t cur = array_pos + 1;
    while (cur < array_end) {
        auto obj_start = text.find('{', cur);
        if (obj_start == std::string::npos || obj_start >= array_end) break;
        auto obj_end = match_bracket(text, obj_start);
        if (obj_end > array_end) break;
        MazeLevel m;
        parse_floor(text.substr(obj_start, obj_end - obj_start), m);
        g_mazes.push_back(std::move(m));
        cur = obj_end;
    }
    std::fprintf(stderr, "[mazes] loaded %zu floors from %s\n",
                 g_mazes.size(), path.c_str());
    return !g_mazes.empty();
}

}  // namespace

const std::vector<MazeLevel>& mazes() {
    if (!g_loaded) {
        g_loaded = true;
#ifdef WIZ_ASSETS_DIR
        load_from_file(std::string(WIZ_ASSETS_DIR) + "/data/wiz1_mazes.json");
#else
        load_from_file("assets/data/wiz1_mazes.json");
#endif
    }
    return g_mazes;
}

}  // namespace wiz::data
