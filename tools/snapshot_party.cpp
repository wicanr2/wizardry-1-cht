// Snapshot the Edge of Town scene with the demo party visible.
// Usage: snapshot_party <out.png>

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include <cstdio>
#include <string>

#include "core/character.h"
#include "core/rules.h"
#include "game/screens.h"
#include "game/state.h"
#include "i18n/tr.h"
#include "render/text.h"
#include "render/ui.h"
#include "render/window.h"

namespace {

void seed(wiz::game::State& s) {
    using namespace wiz::core;
    auto add = [&](const char* name, Race race, Klass k, Alignment a,
                   Attributes attr, int level) {
        auto& c = s.roster.chars[s.roster.used];
        c.name = name; c.race = race; c.klass = k; c.alignment = a;
        c.attr = attr; c.char_level = static_cast<std::uint8_t>(level);
        recompute_derived(c); c.hp_left = c.hp_max;
        c.armor_class = 8; c.gold = 500;
        ++s.roster.used;
    };
    add("阿洛札",  Race::Human, Klass::Fighter, Alignment::Good,    {16,9,10,15,11,10}, 5);
    add("艾琳",    Race::Elf,   Klass::Mage,    Alignment::Neutral, {8,18,10,9,12,10}, 4);
    add("布蘭達",  Race::Hobbit,Klass::Thief,   Alignment::Neutral, {9,10,8,11,17,12}, 4);
    add("西里爾",  Race::Dwarf, Klass::Priest,  Alignment::Good,    {12,9,17,14,9,11}, 4);
    add("德蕾達",  Race::Human, Klass::Bishop,  Alignment::Good,    {11,14,14,12,10,9}, 3);
    add("艾蘿溫",  Race::Elf,   Klass::Samurai, Alignment::Good,    {15,12,11,14,12,10}, 5);
    s.party.count = 6;
    for (int i = 0; i < 6; ++i) s.party.roster_index[i] = i;
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) { std::fprintf(stderr, "usage: snapshot_party <out.png>\n"); return 2; }
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;
    TTF_Init();

    wiz::render::Window window;
    if (!window.create({1280, 720, "snapshot_party"})) return 1;

#ifndef WIZ_ASSETS_DIR
#define WIZ_ASSETS_DIR "."
#endif
#ifndef WIZ_I18N_FILE
#define WIZ_I18N_FILE "src/i18n/strings_zh_tw.json"
#endif
    wiz::i18n::load(WIZ_I18N_FILE);

    const std::string font_path = std::string(WIZ_ASSETS_DIR) + "/fonts/NotoSansCJK-Regular.ttc";
    wiz::render::Font title, body, small;
    title.load(font_path, 36);
    body.load(font_path, 20);
    small.load(font_path, 14);
    wiz::render::UI ui(window.renderer(), &title, &body, &small);

    wiz::game::State state;
    seed(state);
    state.scene = wiz::game::Scene::EdgeOfTown;
    wiz::game::tick(state, nullptr, ui);

    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, 1280, 720, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_RenderReadPixels(window.renderer(), nullptr, SDL_PIXELFORMAT_RGBA32, surf->pixels, surf->pitch);
    IMG_SavePNG(surf, argv[1]);
    SDL_FreeSurface(surf);
    std::fprintf(stderr, "saved: %s\n", argv[1]);

    TTF_Quit();
    SDL_Quit();
    return 0;
}
