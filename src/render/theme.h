#pragma once

#include <string>
#include <string_view>

namespace wiz::render::theme {

// Visual theme — controls monster sprite skin (and later, wall tiles,
// portraits etc.). Cycled with F3 anywhere in the game.
enum class Theme {
    PCECD = 0,   // CC-BY-SA original, full colour (default)
    Mono,        // 1-bit black-and-white (Apple II vector tribute)
    Outline,     // Edge-only line art
    Sepia,       // 4-colour CGA / DOS-era palette
    PC98,        // Local-only: extracted from PC-98 ROM (assets/themes/pc98/)
    WSC,         // Local-only: extracted from WonderSwan Color ROM
    Mac,         // Local-only: extracted from Macintosh disk image
    Count,
};

Theme current();
void set(Theme t);
void cycle();          // current() -> next available (skips missing dirs)
std::string_view display_name(Theme t);
std::string_view dir_name(Theme t);

// Rewrite a sprite-relative path (e.g. "sprites/monsters_pcecd/X.png") to
// the current theme's variant if that file exists under
// "<WIZ_ASSETS_DIR>/themes/<dir>/monsters/X.png", else return the input
// path unchanged so the PCE-CD original is loaded.
std::string resolve(std::string_view rel_path);

// Return the full BGM path for a given scene key ("title", "town", "maze",
// "combat"). Prefers <assets>/themes/<theme>/bgm/<key>.{ogg,mp3} if present,
// else falls back to <assets>/audio/<key>.mp3.
std::string resolve_bgm(std::string_view scene_key);

}  // namespace wiz::render::theme
