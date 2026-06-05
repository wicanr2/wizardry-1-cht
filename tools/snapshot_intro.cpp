// Snapshot one slide of the intro guide.
// Usage: snapshot_intro <out.png> <slide_num 0..4>

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include <cstdio>
#include <string>

#include "game/intro.h"
#include "game/state.h"
#include "i18n/tr.h"
#include "render/text.h"
#include "render/ui.h"
#include "render/window.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: snapshot_intro <out.png> <slide 0..4>\n");
        return 2;
    }
    int slide = std::atoi(argv[2]);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;
    TTF_Init();

    wiz::render::Window window;
    if (!window.create({1280, 720, "snapshot_intro"})) return 1;

#ifndef WIZ_ASSETS_DIR
#define WIZ_ASSETS_DIR "."
#endif
#ifndef WIZ_I18N_FILE
#define WIZ_I18N_FILE "src/i18n/strings_zh_tw.json"
#endif
    wiz::i18n::load(WIZ_I18N_FILE);

    const std::string font = std::string(WIZ_ASSETS_DIR) + "/fonts/NotoSansCJK-Regular.ttc";
    wiz::render::Font title, body, small;
    title.load(font, 36); body.load(font, 20); small.load(font, 14);
    wiz::render::UI ui(window.renderer(), &title, &body, &small);

    wiz::game::State state;
    wiz::game::start_intro();
    // Advance to desired slide by feeding synthetic keypresses
    for (int i = 0; i < slide; ++i) {
        SDL_Event ev{};
        ev.type = SDL_KEYDOWN;
        ev.key.keysym.sym = SDLK_SPACE;
        wiz::game::intro_tick(state, &ev, ui);
    }
    wiz::game::intro_tick(state, nullptr, ui);

    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, 1280, 720, 32, SDL_PIXELFORMAT_RGBA32);
    SDL_RenderReadPixels(window.renderer(), nullptr, SDL_PIXELFORMAT_RGBA32, surf->pixels, surf->pitch);
    IMG_SavePNG(surf, argv[1]);
    SDL_FreeSurface(surf);
    std::fprintf(stderr, "saved: %s (slide %d)\n", argv[1], slide);

    TTF_Quit();
    SDL_Quit();
    return 0;
}
