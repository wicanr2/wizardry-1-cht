#include <SDL.h>
#include <SDL_ttf.h>
#include <sys/stat.h>

#include <cstdio>
#include <ctime>
#include <string>

#include "core/character.h"
#include "core/rules.h"
#include "game/camp.h"
#include "game/screens.h"
#include "game/state.h"
#include "i18n/tr.h"
#include "render/audio.h"
#include "render/text.h"
#include "render/ui.h"
#include "render/window.h"
#include "save/gamesave.h"

namespace {

constexpr int kWidth = 1280;
constexpr int kHeight = 720;

void seed_demo_party(wiz::game::State& state) {
    using namespace wiz::core;
    auto& r = state.roster;

    auto add = [&](const char* name, Race race, Klass klass, Alignment a,
                   Attributes attr, std::uint8_t level) {
        auto& c = r.chars[r.used];
        c.name = name;
        c.race = race;
        c.klass = klass;
        c.alignment = a;
        c.attr = attr;
        c.char_level = level;
        recompute_derived(c);
        recompute_spell_slots(c);
        c.hp_left = c.hp_max;
        c.armor_class = 8;
        c.gold = 500 + level * 100;  // demo starting purse
        c.experience = static_cast<long_t>(level - 1) * 1500;
        ++r.used;
    };

    add("阿洛札",  Race::Human,  Klass::Fighter, Alignment::Good,    {16,9,10,15,11,10}, 5);
    add("艾琳",    Race::Elf,    Klass::Mage,    Alignment::Neutral, {8,18,10,9,12,10},  4);
    add("布蘭達",  Race::Hobbit, Klass::Thief,   Alignment::Neutral, {9,10,8,11,17,12},  4);
    add("西里爾",  Race::Dwarf,  Klass::Priest,  Alignment::Good,    {12,9,17,14,9,11},  4);
    add("德蕾達",  Race::Human,  Klass::Bishop,  Alignment::Good,    {11,14,14,12,10,9}, 3);
    add("艾蘿溫",  Race::Elf,    Klass::Samurai, Alignment::Good,    {15,12,11,14,12,10},5);

    state.party.count = 6;
    for (int i = 0; i < 6; ++i) state.party.roster_index[i] = i;
}

}  // namespace

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() != 0) {
        std::fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    int rc = 0;
    {
    wiz::render::Window window;
    // Note: SDL window title is rendered by the host window manager which
    // often doesn't handle UTF-8 well. Keep ASCII to avoid garbled titlebars.
    if (!window.create({kWidth, kHeight, "Wizardry I CHT (Proving Grounds of the Mad Overlord)"})) {
        TTF_Quit(); SDL_Quit(); return 1;
    }

#ifndef WIZ_ASSETS_DIR
#define WIZ_ASSETS_DIR "."
#endif
#ifndef WIZ_I18N_FILE
#define WIZ_I18N_FILE "src/i18n/strings_zh_tw.json"
#endif

    wiz::i18n::load(WIZ_I18N_FILE);
    wiz::render::audio_init();

    const std::string font_path = std::string(WIZ_ASSETS_DIR) + "/fonts/NotoSansCJK-Regular.ttc";
    wiz::render::Font title_font, body_font, small_font;
    if (!title_font.load(font_path, 36) ||
        !body_font.load(font_path, 20) ||
        !small_font.load(font_path, 14)) {
        std::fprintf(stderr, "font load failed (%s)\n", font_path.c_str());
        TTF_Quit(); SDL_Quit(); return 1;
    }

    wiz::render::UI ui(window.renderer(), &title_font, &body_font, &small_font);

    wiz::game::State state;
    // Pick the newest existing save slot (mtime); fall back to slot 1 path.
    std::string save_path = wiz::game::default_save_path();
    int newest_slot = -1;
    {
        time_t newest = 0;
        for (int i = 1; i <= wiz::game::kNumSlots; ++i) {
            std::string p = wiz::game::save_path_for_slot(i);
            struct stat st;
            if (stat(p.c_str(), &st) == 0 && st.st_mtime >= newest) {
                newest = st.st_mtime;
                save_path = p;
                newest_slot = i;
            }
        }
    }
    if (newest_slot > 0 && wiz::save::load_game(state, save_path)) {
        char buf[200];
        std::snprintf(buf, sizeof(buf),
                      "讀取存檔 Slot %d：%s", newest_slot, save_path.c_str());
        state.push_message(buf);
        state.push_message("巫術繁中版 — 歡迎回來。在標題畫面按 1-5 可切換存檔槽。");
    } else {
        seed_demo_party(state);
        state.push_message("巫術繁中版開機完成（新遊戲）。");
        state.push_message("Tip: 在迷宮按 C 進營地存檔；標題畫面按 1-5 切槽。");
    }

    bool running = true;
    while (running) {
        SDL_Event ev;
        while (running && SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { running = false; break; }
            running = wiz::game::tick(state, &ev, ui);
        }
        if (running) wiz::game::tick(state, nullptr, ui);
        SDL_Delay(16);
    }

    }  // Window + Fonts destroyed before TTF/SDL teardown

    wiz::render::audio_shutdown();
    TTF_Quit();
    SDL_Quit();
    return rc;
}
