#include "i18n/tr.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace wiz::i18n {

namespace {

struct Entry {
    std::string zh_tw;
    std::string en;
};

std::unordered_map<std::string, Entry> g_table;

// Tiny JSON value extractor for the catalogue shape:
//   { "key": { "en": "...", "zh_TW": "...", "category": "...", "refs": [...] }, ... }
// We avoid a full JSON dependency at runtime for the loader so wizcore stays slim.
std::string read_string_field(const std::string& obj, const std::string& field) {
    auto pos = obj.find("\"" + field + "\"");
    if (pos == std::string::npos) return {};
    pos = obj.find(':', pos);
    if (pos == std::string::npos) return {};
    pos = obj.find('"', pos);
    if (pos == std::string::npos) return {};
    ++pos;
    std::string out;
    while (pos < obj.size()) {
        char c = obj[pos];
        if (c == '\\' && pos + 1 < obj.size()) {
            char n = obj[pos + 1];
            switch (n) {
                case '"': out.push_back('"'); break;
                case '\\': out.push_back('\\'); break;
                case 'n': out.push_back('\n'); break;
                case 't': out.push_back('\t'); break;
                case '/': out.push_back('/'); break;
                default: out.push_back(n); break;
            }
            pos += 2;
        } else if (c == '"') {
            break;
        } else {
            out.push_back(c);
            ++pos;
        }
    }
    return out;
}

}  // namespace

bool load(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        std::fprintf(stderr, "[i18n] cannot open %s\n", path.c_str());
        return false;
    }
    std::stringstream buf;
    buf << in.rdbuf();
    std::string text = buf.str();

    // Iterate top-level keys: "key": { ... }
    std::size_t pos = 0;
    while (pos < text.size()) {
        auto q1 = text.find('"', pos);
        if (q1 == std::string::npos) break;
        auto q2 = text.find('"', q1 + 1);
        if (q2 == std::string::npos) break;
        std::string key = text.substr(q1 + 1, q2 - q1 - 1);
        pos = q2 + 1;

        auto colon = text.find(':', pos);
        if (colon == std::string::npos) break;
        auto next = text.find_first_not_of(" \t\r\n", colon + 1);
        if (next == std::string::npos) break;

        if (text[next] != '{') {
            // Skip non-object values (e.g. _meta scalars).
            auto endline = text.find_first_of(",}\n", next);
            pos = (endline == std::string::npos) ? text.size() : endline + 1;
            continue;
        }

        int depth = 1;
        std::size_t end = next + 1;
        while (end < text.size() && depth > 0) {
            char c = text[end];
            if (c == '"') {
                // Skip string
                ++end;
                while (end < text.size() && text[end] != '"') {
                    if (text[end] == '\\' && end + 1 < text.size()) end += 2;
                    else ++end;
                }
                if (end < text.size()) ++end;
                continue;
            }
            if (c == '{') ++depth;
            else if (c == '}') --depth;
            ++end;
        }
        std::string obj = text.substr(next, end - next);
        pos = end;

        if (key == "_meta") continue;

        Entry e;
        e.en = read_string_field(obj, "en");
        e.zh_tw = read_string_field(obj, "zh_TW");
        g_table.emplace(std::move(key), std::move(e));
    }
    std::fprintf(stderr, "[i18n] loaded %zu entries from %s\n", g_table.size(), path.c_str());
    return !g_table.empty();
}

std::string_view tr(std::string_view key) {
    auto it = g_table.find(std::string{key});
    if (it == g_table.end()) return key;
    return it->second.zh_tw.empty() ? std::string_view{it->second.en}
                                    : std::string_view{it->second.zh_tw};
}

std::string_view tr_en(std::string_view key) {
    auto it = g_table.find(std::string{key});
    return (it == g_table.end()) ? key : std::string_view{it->second.en};
}

}  // namespace wiz::i18n
