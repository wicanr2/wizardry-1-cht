#include "render/window.h"

#include <SDL.h>

#include <cstdio>

namespace wiz::render {

Window::~Window() { destroy(); }

bool Window::create(const WindowOptions& opts) {
    opts_ = opts;
    window_ = SDL_CreateWindow(
        opts.title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        opts.width, opts.height, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window_) {
        std::fprintf(stderr, "[window] SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }
    renderer_ = SDL_CreateRenderer(
        window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) {
        std::fprintf(stderr, "[window] SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        return false;
    }
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    return true;
}

void Window::destroy() {
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
}

}  // namespace wiz::render
