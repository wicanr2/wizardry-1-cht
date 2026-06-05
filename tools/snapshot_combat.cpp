// Snapshot tool: forces a combat scene and saves a PNG snapshot, bypassing
// xdotool focus issues. Builds as a separate executable that links wizgame.
//
// Usage: snapshot_combat <output.png> [monster_id]

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include <cstdio>
#include <cstring>
#include <string>

#include "core/rng.h"
#include "core/rules.h"
#include "data/items_db.h"
#include "game/screens.h"
#include "game/state.h"
#include "i18n/tr.h"
#include "render/audio.h"
#include "render/sprite.h"
#include "render/text.h"
#include "render/ui.h"
#include "render/window.h"

namespace {

void seed_demo(wiz::game::State& s) {
    using namespace wiz::core;
    auto add = [&](const char* nm, Race race, Klass k, Alignment a,
                   Attributes attr, int level) {
        auto& c = s.roster.chars[s.roster.used];
        c.name = nm; c.race = race; c.klass = k; c.alignment = a;
        c.attr = attr; c.char_level = static_cast<std::uint8_t>(level);
        recompute_derived(c);
        c.hp_left = c.hp_max;
        c.armor_class = 8;
        c.gold = 500;
        ++s.roster.used;
    };
    add("ALOZAR",  Race::Human, Klass::Fighter, Alignment::Good,
        {16,9,10,15,11,10}, 5);
    add("ARYNN",   Race::Elf,   Klass::Mage,    Alignment::Neutral,
        {8,18,10,9,12,10}, 4);
    add("CYRIL",   Race::Dwarf, Klass::Priest,  Alignment::Good,
        {12,9,17,14,9,11}, 4);
    s.party.count = 3;
    for (int i = 0; i < 3; ++i) s.party.roster_index[i] = i;
}

void save_snapshot(SDL_Renderer* r, int w, int h, const char* path) {
    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32,
                                                       SDL_PIXELFORMAT_RGBA32);
    SDL_RenderReadPixels(r, nullptr, SDL_PIXELFORMAT_RGBA32, surf->pixels, surf->pitch);
    IMG_SavePNG(surf, path);
    SDL_FreeSurface(surf);
    std::fprintf(stderr, "saved snapshot: %s\n", path);
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: snapshot_combat <output.png> [monster_id]\n");
        return 2;
    }
    const char* out_path = argv[1];
    int monster_id = (argc > 2) ? std::atoi(argv[2]) : -1;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;
    TTF_Init();

    wiz::render::Window window;
    if (!window.create({1280, 720, "snapshot"})) return 1;

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
    seed_demo(state);

    // Pick the monster
    using namespace wiz::core;
    if (monster_id < 0) monster_id = 25;  // default: Werdna
    const auto* entry = wiz::data::find_monster_by_id(monster_id);
    if (!entry) {
        std::fprintf(stderr, "monster id %d not found\n", monster_id);
        return 1;
    }

    Monster proto;
    proto.name = entry->name_zh.empty() ? entry->name_en : entry->name_zh;
    proto.name_unknown = entry->name_unknown.empty() ? entry->name_en : entry->name_unknown;
    proto.armor_class = static_cast<std::int8_t>(entry->ac);
    proto.experience = entry->exp;
    proto.sprite_path = entry->sprite_path;
    proto.data_id = entry->id;
    auto pos = entry->hp_dice.find('d');
    if (pos != std::string::npos) {
        proto.hp_dice_n = static_cast<std::uint8_t>(std::stoi(entry->hp_dice.substr(0, pos)));
        proto.hp_dice_d = static_cast<std::uint8_t>(std::stoi(entry->hp_dice.substr(pos + 1)));
    }

    CombatGroup g;
    g.prototype = proto;
    g.total_count = 2;
    begin_combat(state.combat, {g});
    state.combat.active_party_member = 0;
    PlayerAction parry; parry.kind = PlayerAction::Parry;
    for (int i = 0; i < 6; ++i) set_action(state.combat, i, parry);

    state.scene = wiz::game::Scene::Combat;

    // Drive 1 tick to render
    wiz::game::tick(state, nullptr, ui);
    SDL_Delay(100);
    wiz::game::tick(state, nullptr, ui);

    save_snapshot(window.renderer(), 1280, 720, out_path);

    TTF_Quit();
    SDL_Quit();
    return 0;
}
