// Snapshot the help overlay in a specific scene.
// Usage: snapshot_help <out.png> <scene_num>
//   scene_num: 0=Title, 1=Edge, 2=Castle, 3=Maze, 4=Combat, 5=Camp, 6=Shop, 7=Tavern

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include <cstdio>
#include <string>

#include "game/help.h"
#include "game/screens.h"
#include "game/state.h"
#include "i18n/tr.h"
#include "render/text.h"
#include "render/ui.h"
#include "render/window.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: snapshot_help <out.png> <scene>\n");
        return 2;
    }
    const char* out_path = argv[1];
    int scene_n = std::atoi(argv[2]);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;
    TTF_Init();

    wiz::render::Window window;
    if (!window.create({1280, 720, "snapshot_help"})) return 1;

#ifndef WIZ_ASSETS_DIR
#define WIZ_ASSETS_DIR "."
#endif
#ifndef WIZ_I18N_FILE
#define WIZ_I18N_FILE "src/i18n/strings_zh_tw.json"
#endif

    wiz::i18n::load(WIZ_I18N_FILE);
    const std::string font_path =
        std::string(WIZ_ASSETS_DIR) + "/fonts/NotoSansCJK-Regular.ttc";
    wiz::render::Font title, body, small;
    title.load(font_path, 36);
    body.load(font_path, 20);
    small.load(font_path, 14);
    wiz::render::UI ui(window.renderer(), &title, &body, &small);

    wiz::game::State state;
    static const wiz::game::Scene kScenes[] = {
        wiz::game::Scene::Title,
        wiz::game::Scene::EdgeOfTown,
        wiz::game::Scene::Castle,
        wiz::game::Scene::Maze,
        wiz::game::Scene::Combat,
        wiz::game::Scene::Camp,
        wiz::game::Scene::Shop,
        wiz::game::Scene::Tavern,
    };
    if (scene_n < 0 || scene_n >= static_cast<int>(std::size(kScenes))) scene_n = 0;
    state.scene = kScenes[scene_n];

    // Trigger help on
    wiz::game::toggle_help();
    wiz::game::tick(state, nullptr, ui);

    SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat(0, 1280, 720, 32,
                                                       SDL_PIXELFORMAT_RGBA32);
    SDL_RenderReadPixels(window.renderer(), nullptr,
                         SDL_PIXELFORMAT_RGBA32, surf->pixels, surf->pitch);
    IMG_SavePNG(surf, out_path);
    SDL_FreeSurface(surf);
    std::fprintf(stderr, "saved: %s (scene %d)\n", out_path, scene_n);

    TTF_Quit();
    SDL_Quit();
    return 0;
}
