#pragma once

#include <SDL.h>

#include "core/maze.h"
#include "render/maze_view.h"
#include "render/ui.h"

namespace wiz::render {

// Eye-of-map style auto-map: 20x20 grid panel showing visited cells, walls
// for cells the camera has revealed, and the current camera position.
void draw_auto_map(SDL_Renderer* r, const core::MazeLevel& m,
                   const Camera& cam, SDL_Rect rect, const Theme& theme);

// Mark the cells visible from a camera position as visited (the cell itself
// plus a fan of cells the player can see).
void reveal_from(core::MazeLevel& m, const Camera& cam) noexcept;

}  // namespace wiz::render
