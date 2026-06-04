#include "render/sprite.h"

#include <SDL.h>
#include <SDL_image.h>

#include <cstdio>
#include <string>
#include <unordered_map>

namespace wiz::render {

namespace {

std::unordered_map<std::string, SDL_Texture*> g_cache;

}  // namespace

SDL_Texture* load_sprite(SDL_Renderer* r, std::string_view path) {
    if (path.empty()) {
        clear_sprite_cache();
        return nullptr;
    }
    std::string key{path};
    auto it = g_cache.find(key);
    if (it != g_cache.end()) return it->second;

    SDL_Texture* tex = IMG_LoadTexture(r, key.c_str());
    if (!tex) {
        std::fprintf(stderr, "[sprite] cannot load %s: %s\n",
                     key.c_str(), IMG_GetError());
        g_cache[key] = nullptr;  // negative cache, don't retry
        return nullptr;
    }
    g_cache[key] = tex;
    return tex;
}

void clear_sprite_cache() {
    for (auto& [k, t] : g_cache) if (t) SDL_DestroyTexture(t);
    g_cache.clear();
}

void draw_sprite_fit(SDL_Renderer* r, SDL_Texture* tex, SDL_Rect dst) {
    if (!tex) return;
    int w = 0, h = 0;
    SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
    if (w == 0 || h == 0) return;
    // Preserve aspect: fit inside dst, centered.
    float scale = std::min(float(dst.w) / w, float(dst.h) / h);
    int sw = static_cast<int>(w * scale);
    int sh = static_cast<int>(h * scale);
    SDL_Rect fit{dst.x + (dst.w - sw) / 2, dst.y + (dst.h - sh) / 2, sw, sh};
    SDL_RenderCopy(r, tex, nullptr, &fit);
}

}  // namespace wiz::render
