#include "save/roster.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

namespace wiz::save {

namespace {

std::string escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\t': out += "\\t"; break;
            default: out.push_back(c); break;
        }
    }
    return out;
}

void write_char(std::ostream& os, const core::Character& c) {
    os << "{\n";
    os << R"(  "name":")" << escape(c.name) << "\",\n";
    os << R"(  "password":")" << escape(c.password) << "\",\n";
    os << R"(  "in_maze":)" << (c.in_maze ? "true" : "false") << ",\n";
    os << R"(  "race":)" << int(c.race) << ",\n";
    os << R"(  "klass":)" << int(c.klass) << ",\n";
    os << R"(  "alignment":)" << int(c.alignment) << ",\n";
    os << R"(  "status":)" << int(c.status) << ",\n";
    os << R"(  "age":)" << c.age << ",\n";
    os << R"(  "attr":{"str":)" << int(c.attr.strength)
       << R"(,"iq":)" << int(c.attr.iq)
       << R"(,"pie":)" << int(c.attr.piety)
       << R"(,"vit":)" << int(c.attr.vitality)
       << R"(,"agi":)" << int(c.attr.agility)
       << R"(,"luk":)" << int(c.attr.luck)
       << "},\n";
    os << R"(  "gold":)" << c.gold << ",\n";
    os << R"(  "exp":)" << c.experience << ",\n";
    os << R"(  "char_level":)" << int(c.char_level) << ",\n";
    os << R"(  "hp_left":)" << c.hp_left << ",\n";
    os << R"(  "hp_max":)" << c.hp_max << ",\n";
    os << R"(  "ac":)" << int(c.armor_class) << "\n";
    os << "}";
}

// Tiny field reader, same approach as i18n loader.
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
    auto end = obj.find_first_of(",}\n", first);
    try { return std::stoll(obj.substr(first, end - first)); }
    catch (...) { return def; }
}

bool bool_field(const std::string& obj, const std::string& key) {
    auto pos = obj.find("\"" + key + "\"");
    if (pos == std::string::npos) return false;
    pos = obj.find(':', pos);
    auto first = obj.find_first_not_of(" \t\n", pos + 1);
    return obj.compare(first, 4, "true") == 0;
}

}  // namespace

bool save(const Roster& roster, const std::string& path) {
    std::ofstream out(path);
    if (!out) {
        std::fprintf(stderr, "[save] cannot write %s\n", path.c_str());
        return false;
    }
    out << "{\n";
    out << R"(  "version": 1,)" << "\n";
    out << R"(  "used": )" << roster.used << ",\n";
    out << R"(  "chars": [)" << "\n";
    for (std::size_t i = 0; i < roster.used; ++i) {
        write_char(out, roster.chars[i]);
        if (i + 1 < roster.used) out << ",";
        out << "\n";
    }
    out << "  ]\n}\n";
    return true;
}

bool load(Roster& roster, const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        std::fprintf(stderr, "[save] cannot open %s\n", path.c_str());
        return false;
    }
    std::stringstream buf; buf << in.rdbuf();
    const std::string text = buf.str();

    roster.used = 0;
    std::size_t pos = text.find("\"chars\"");
    if (pos == std::string::npos) return false;
    pos = text.find('[', pos);
    if (pos == std::string::npos) return false;

    while (pos < text.size() && roster.used < Roster::kMaxChars) {
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

        core::Character c;
        c.name = str_field(obj, "name");
        c.password = str_field(obj, "password");
        c.in_maze = bool_field(obj, "in_maze");
        c.race = static_cast<core::Race>(int_field(obj, "race"));
        c.klass = static_cast<core::Klass>(int_field(obj, "klass"));
        c.alignment = static_cast<core::Alignment>(int_field(obj, "alignment"));
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
        c.char_level = static_cast<std::uint8_t>(int_field(obj, "char_level", 1));
        c.hp_left = static_cast<std::int16_t>(int_field(obj, "hp_left"));
        c.hp_max = static_cast<std::int16_t>(int_field(obj, "hp_max"));
        c.armor_class = static_cast<std::uint8_t>(int_field(obj, "ac", 10));

        if (!c.name.empty()) {
            roster.chars[roster.used++] = std::move(c);
        }

        auto close = text.find(']', pos);
        if (close != std::string::npos && close < text.find('{', pos)) break;
    }
    return true;
}

}  // namespace wiz::save
