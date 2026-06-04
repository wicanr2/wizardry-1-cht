#include "core/spell.h"

#include <cstdio>
#include <fstream>
#include <sstream>

namespace wiz::core {

namespace {

// Mini JSON reader — same trick as i18n/tr.cpp to keep wizcore dependency-light.
std::string field(const std::string& obj, const std::string& key) {
    auto pos = obj.find("\"" + key + "\"");
    if (pos == std::string::npos) return {};
    pos = obj.find(':', pos);
    if (pos == std::string::npos) return {};
    auto first = obj.find_first_not_of(" \t\n", pos + 1);
    if (first == std::string::npos) return {};
    if (obj[first] == '"') {
        auto end = obj.find('"', first + 1);
        return obj.substr(first + 1, end - first - 1);
    }
    auto end = obj.find_first_of(",}\n", first);
    return obj.substr(first, end - first);
}

int field_int(const std::string& obj, const std::string& key, int def = 0) {
    auto s = field(obj, key);
    if (s.empty()) return def;
    try { return std::stoi(s); } catch (...) { return def; }
}

}  // namespace

bool SpellBook::load(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        std::fprintf(stderr, "[spell] cannot open %s\n", path.c_str());
        return false;
    }
    std::stringstream buf; buf << in.rdbuf();
    std::string text = buf.str();

    count_ = 0;
    std::size_t pos = 0;
    while (pos < text.size() && count_ < spells_.size()) {
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

        Spell s;
        s.name = field(obj, "name");
        s.description = field(obj, "description");
        s.school = field(obj, "school") == "priest" ? SpellSchool::Priest : SpellSchool::Mage;
        s.level = static_cast<std::uint8_t>(field_int(obj, "level", 1));
        s.group = static_cast<std::uint8_t>(field_int(obj, "group", 0));
        s.type = static_cast<std::uint8_t>(field_int(obj, "type", 0));
        s.combat_only = field(obj, "combat_only") == "true";
        s.camp_only = field(obj, "camp_only") == "true";

        if (!s.name.empty()) {
            spells_[count_++] = std::move(s);
        }
    }
    std::fprintf(stderr, "[spell] loaded %zu spells\n", count_);
    return count_ > 0;
}

const Spell* SpellBook::find(std::string_view name) const noexcept {
    for (std::size_t i = 0; i < count_; ++i) {
        if (spells_[i].name == name) return &spells_[i];
    }
    return nullptr;
}

const Spell* SpellBook::at(std::size_t idx) const noexcept {
    return (idx < count_) ? &spells_[idx] : nullptr;
}

SpellBook& global_spellbook() noexcept {
    static SpellBook book;
    return book;
}

}  // namespace wiz::core
