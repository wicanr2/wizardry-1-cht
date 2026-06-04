#pragma once

#include <SDL.h>

#include <string>
#include <string_view>

namespace wiz::render {

// Lazy-loaded sprite cache. PNG files are read via SDL_image once on first
// use, then cached. Pass empty string_view to release everything.
SDL_Texture* load_sprite(SDL_Renderer* r, std::string_view path);
void clear_sprite_cache();

// Convenience: render sprite scaled to fit a target rect, preserving aspect.
void draw_sprite_fit(SDL_Renderer* r, SDL_Texture* tex, SDL_Rect dst);

}  // namespace wiz::render
