#include "render/theme.h"

#include <sys/stat.h>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <string_view>

#ifndef WIZ_ASSETS_DIR
#define WIZ_ASSETS_DIR "assets"
#endif

namespace wiz::render::theme {
namespace {

Theme g_current = Theme::PCECD;

struct Info {
    const char* display;
    const char* dir;       // empty for PCECD passthrough
};

constexpr std::array<Info, static_cast<size_t>(Theme::Count)> kInfo = {{
    {"PCE-CD (彩色原版)",       ""},
    {"Mono (Apple II 黑白致敬)", "mono"},
    {"Outline (Apple II 線稿)",  "outline"},
    {"Sepia (CGA / DOS 風)",     "sepia"},
    {"PC-98 (本機)",              "pc98"},
    {"WonderSwan Color (本機)",   "wsc"},
    {"Macintosh (本機)",          "mac"},
}};

bool dir_exists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

bool file_exists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

}  // namespace

Theme current() { return g_current; }
void  set(Theme t) {
    if (static_cast<int>(t) < 0 ||
        static_cast<int>(t) >= static_cast<int>(Theme::Count)) {
        return;
    }
    g_current = t;
}

void cycle() {
    int n = static_cast<int>(Theme::Count);
    int next = (static_cast<int>(g_current) + 1) % n;
    // Skip themes whose directory is missing (e.g. ROM-extracted themes
    // the user hasn't populated). PCECD has no dir requirement.
    for (int tries = 0; tries < n; ++tries) {
        Theme cand = static_cast<Theme>(next);
        if (cand == Theme::PCECD) { g_current = cand; return; }
        std::string dir = std::string(WIZ_ASSETS_DIR) + "/themes/" +
                          kInfo[static_cast<size_t>(cand)].dir;
        if (dir_exists(dir)) { g_current = cand; return; }
        next = (next + 1) % n;
    }
    g_current = Theme::PCECD;
}

std::string_view display_name(Theme t) {
    return kInfo[static_cast<size_t>(t)].display;
}

std::string_view dir_name(Theme t) {
    return kInfo[static_cast<size_t>(t)].dir;
}

std::string resolve(std::string_view rel_path) {
    // PCECD = passthrough.
    if (g_current == Theme::PCECD) return std::string(rel_path);

    // Only rewrite paths under sprites/monsters_pcecd/.
    constexpr std::string_view kPrefix = "sprites/monsters_pcecd/";
    if (rel_path.size() < kPrefix.size() ||
        rel_path.substr(0, kPrefix.size()) != kPrefix) {
        return std::string(rel_path);
    }

    std::string_view fname = rel_path.substr(kPrefix.size());
    const auto& info = kInfo[static_cast<size_t>(g_current)];
    std::string themed = std::string("themes/") + info.dir + "/monsters/" +
                         std::string(fname);
    std::string full = std::string(WIZ_ASSETS_DIR) + "/" + themed;
    if (file_exists(full)) return themed;
    return std::string(rel_path);
}

}  // namespace wiz::render::theme
