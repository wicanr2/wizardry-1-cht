#pragma once

#include <SDL.h>

#include "core/maze.h"
#include "render/ui.h"

namespace wiz::render {

enum class Facing : std::uint8_t { North = 0, East = 1, South = 2, West = 3 };

struct Camera {
    int x = 0;
    int y = 0;
    int level = 1;
    Facing facing = Facing::North;
};

// Draws a wireframe 3D view of the maze from camera position into rect.
// Replicates RUNNER.TEXT DRAWMAZE behaviour: looks up to 4 cells ahead,
// renders trapezoidal corridor walls + doors using SDL_RenderDrawLine.
void draw_maze_view(SDL_Renderer* r, const core::MazeLevel& level,
                    const Camera& cam, SDL_Rect viewport, const Theme& theme);

// Helper to step a camera one cell forward / strafe / turn.
void step_forward(Camera& cam) noexcept;
void step_back(Camera& cam) noexcept;
void turn_left(Camera& cam) noexcept;
void turn_right(Camera& cam) noexcept;

// Reads the wall in front of (x, y) when facing `f`. Used by collision check.
core::Wall front_wall(const core::MazeLevel& m, int x, int y, Facing f) noexcept;

}  // namespace wiz::render
