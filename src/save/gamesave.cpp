#include "save/gamesave.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

#include "save/roster.h"

namespace wiz::save {

namespace {

std::string escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            default:   out.push_back(c); break;
        }
    }
    return out;
}

std::string str_field(const std::string& obj, const std::string& key) {
    auto pos = obj.find("\"" + key + "\"");
    if (pos == std::string::npos) return {};
    pos = obj.find(':', pos);
    pos = obj.find('"', pos);
    if (pos == std::string::npos) return {};
    ++pos;
    std::string out;
    while (pos < obj.size() && obj[pos] != '"') {
        if (obj[pos] == '\\' && pos + 1 < obj.size()) {
            char n = obj[pos + 1];
            out.push_back(n == 'n' ? '\n' : n);
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
    if (pos == std::string::npos) return def;
    auto first = obj.find_first_not_of(" \t\n", pos + 1);
    auto end = obj.find_first_of(",}\n]", first);
    try { return std::stoll(obj.substr(first, end - first)); }
    catch (...) { return def; }
}

}  // namespace

bool save_game(const game::State& state, const std::string& path) {
    std::ofstream out(path);
    if (!out) {
        std::fprintf(stderr, "[save] cannot open %s for write\n", path.c_str());
        return false;
    }
    out << "{\n";
    out << R"(  "version": 2,)" << "\n";
    out << R"(  "roster": {)" << "\n";
    out << R"(    "used": )" << state.roster.used << ",\n";
    out << R"(    "chars": [)" << "\n";
    for (std::size_t i = 0; i < state.roster.used; ++i) {
        const auto& c = state.roster.chars[i];
        out << "      {";
        out << R"("name":")" << escape(c.name) << "\","
            << R"("race":)" << int(c.race) << ","
            << R"("klass":)" << int(c.klass) << ","
            << R"("align":)" << int(c.alignment) << ","
            << R"("status":)" << int(c.status) << ","
            << R"("age":)" << c.age << ","
            << R"("str":)"  << int(c.attr.strength) << ","
            << R"("iq":)"   << int(c.attr.iq) << ","
            << R"("pie":)"  << int(c.attr.piety) << ","
            << R"("vit":)"  << int(c.attr.vitality) << ","
            << R"("agi":)"  << int(c.attr.agility) << ","
            << R"("luk":)"  << int(c.attr.luck) << ","
            << R"("gold":)" << c.gold << ","
            << R"("exp":)"  << c.experience << ","
            << R"("lvl":)"  << int(c.char_level) << ","
            << R"("hp":)"   << c.hp_left << ","
            << R"("hpmax":)" << c.hp_max << ","
            << R"("ac":)"   << int(c.armor_class);
        out << "}";
        if (i + 1 < state.roster.used) out << ",";
        out << "\n";
    }
    out << "    ]\n  },\n";

    out << R"(  "party": {)" << "\n";
    out << R"(    "count": )" << state.party.count << ",\n";
    out << R"(    "ids": [)";
    for (int i = 0; i < state.party.count; ++i) {
        out << state.party.roster_index[i];
        if (i + 1 < state.party.count) out << ", ";
    }
    out << "]\n  },\n";

    out << R"(  "camera": {)" << "\n";
    out << R"(    "x": )" << state.camera.x << ","
        << R"( "y": )" << state.camera.y << ","
        << R"( "level": )" << state.camera.level << ","
        << R"( "facing": )" << int(state.camera.facing) << ",\n";
    out << R"(    "maze_loaded": )" << (state.maze_loaded ? "true" : "false") << "\n  }\n";
    out << "}\n";
    return true;
}

bool load_game(game::State& state, const std::string& path) {
    std::ifstream in(path);
    if (!in) return false;
    std::stringstream buf;
    buf << in.rdbuf();
    const std::string text = buf.str();

    // Parse roster
    state.roster.used = 0;
    auto roster_pos = text.find("\"roster\"");
    if (roster_pos == std::string::npos) return false;
    auto chars_pos = text.find("\"chars\"", roster_pos);
    if (chars_pos == std::string::npos) return false;
    auto arr_start = text.find('[', chars_pos);
    auto arr_end = text.find(']', arr_start);
    std::size_t pos = arr_start + 1;
    while (pos < arr_end && state.roster.used < Roster::kMaxChars) {
        auto open = text.find('{', pos);
        if (open == std::string::npos || open > arr_end) break;
        int depth = 1;
        auto close = open + 1;
        while (close < text.size() && depth > 0) {
            if (text[close] == '{') ++depth;
            else if (text[close] == '}') --depth;
            ++close;
        }
        std::string obj = text.substr(open, close - open);
        pos = close;

        core::Character c;
        c.name = str_field(obj, "name");
        c.race = static_cast<core::Race>(int_field(obj, "race"));
        c.klass = static_cast<core::Klass>(int_field(obj, "klass"));
        c.alignment = static_cast<core::Alignment>(int_field(obj, "align"));
        c.status = static_cast<core::Status>(int_field(obj, "status"));
        c.age = static_cast<std::uint16_t>(int_field(obj, "age"));
        c.attr.strength = static_cast<std::uint8_t>(int_field(obj, "str"));
        c.attr.iq = static_cast<std::uint8_t>(int_field(obj, "iq"));
        c.attr.piety = static_cast<std::uint8_t>(int_field(obj, "pie"));
        c.attr.vitality = static_cast<std::uint8_t>(int_field(obj, "vit"));
        c.attr.agility = static_cast<std::uint8_t>(int_field(obj, "agi"));
        c.attr.luck = static_cast<std::uint8_t>(int_field(obj, "luk"));
        c.gold = int_field(obj, "gold");
        c.experience = int_field(obj, "exp");
        c.char_level = static_cast<std::uint8_t>(int_field(obj, "lvl", 1));
        c.hp_left = static_cast<std::int16_t>(int_field(obj, "hp"));
        c.hp_max = static_cast<std::int16_t>(int_field(obj, "hpmax"));
        c.armor_class = static_cast<std::uint8_t>(int_field(obj, "ac", 10));
        if (!c.name.empty()) state.roster.chars[state.roster.used++] = std::move(c);
    }

    // Parse party
    state.party.count = 0;
    for (int i = 0; i < 6; ++i) state.party.roster_index[i] = -1;
    auto party_pos = text.find("\"party\"");
    if (party_pos != std::string::npos) {
        state.party.count = static_cast<int>(int_field(text.substr(party_pos), "count"));
        auto ids_pos = text.find("\"ids\"", party_pos);
        if (ids_pos != std::string::npos) {
            auto open = text.find('[', ids_pos);
            auto close = text.find(']', open);
            std::string ids = text.substr(open + 1, close - open - 1);
            std::stringstream ss(ids);
            int idx = 0, v;
            char comma;
            while (ss >> v && idx < 6) {
                state.party.roster_index[idx++] = v;
                ss >> comma;
            }
            if (state.party.count > idx) state.party.count = idx;
        }
    }

    // Parse camera
    auto cam_pos = text.find("\"camera\"");
    if (cam_pos != std::string::npos) {
        std::string cam = text.substr(cam_pos);
        state.camera.x = static_cast<int>(int_field(cam, "x"));
        state.camera.y = static_cast<int>(int_field(cam, "y"));
        state.camera.level = static_cast<int>(int_field(cam, "level", 1));
        state.camera.facing = static_cast<render::Facing>(int_field(cam, "facing"));
        state.maze_loaded = cam.find("\"maze_loaded\": true") != std::string::npos;
    }
    return true;
}

}  // namespace wiz::save
