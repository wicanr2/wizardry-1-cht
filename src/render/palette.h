#pragma once

#include <SDL.h>

#include "render/theme.h"

namespace wiz::render {

// Per-theme dungeon palette. Drives wall / door / floor / ceiling / frame
// colours of the 3D maze view so that F3 (visual theme cycle) actually
// changes how the dungeon looks, not just monster sprites.
struct MazePalette {
    SDL_Color wall;       // Stone wall lines.
    SDL_Color door;       // Door frames.
    SDL_Color frame;      // Outer viewport border.
    SDL_Color floor;      // Floor fill (bottom half).
    SDL_Color ceiling;    // Ceiling fill (top half).
    SDL_Color far_wall;   // Optional accent for the terminal "wall ahead" rect.
};

MazePalette maze_palette_for(theme::Theme t);

// Convenience: returns the palette for the currently active theme.
inline MazePalette current_maze_palette() {
    return maze_palette_for(theme::current());
}

}  // namespace wiz::render
