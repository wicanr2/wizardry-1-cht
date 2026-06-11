#include "render/maze_view.h"

#include <algorithm>
#include <array>

namespace wiz::render {

namespace {

// Direction vectors indexed by Facing.
constexpr int kDx[4] = {0, 1, 0, -1};   // N, E, S, W
constexpr int kDy[4] = {-1, 0, 1, 0};

// Returns the wall on the given side of cell (x, y) when facing `f`.
// side: 0 = front, 1 = right, 2 = left.
core::Wall get_wall(const core::MazeLevel& m, int x, int y, Facing f, int side) {
    if (x < 0 || x >= core::MazeLevel::kSize ||
        y < 0 || y >= core::MazeLevel::kSize) {
        return core::Wall::Wall;
    }
    // Map (facing, side) to absolute wall direction.
    static constexpr int kRotation[4][3] = {
        // front,right,left  for N,E,S,W
        {0, 1, 3},  // N: front=N, right=E, left=W
        {1, 2, 0},  // E: front=E, right=S, left=N
        {2, 3, 1},  // S: front=S, right=W, left=E
        {3, 0, 2},  // W: front=W, right=N, left=S
    };
    int dir = kRotation[static_cast<int>(f)][side];
    switch (dir) {
        case 0: return m.north[y][x];
        case 1: return m.east[y][x];
        case 2: return m.south[y][x];
        case 3: return m.west[y][x];
    }
    return core::Wall::Wall;
}

void set_color(SDL_Renderer* r, SDL_Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, 255);
}

void draw_rect(SDL_Renderer* r, int x1, int y1, int x2, int y2) {
    SDL_RenderDrawLine(r, x1, y1, x2, y1);
    SDL_RenderDrawLine(r, x2, y1, x2, y2);
    SDL_RenderDrawLine(r, x2, y2, x1, y2);
    SDL_RenderDrawLine(r, x1, y2, x1, y1);
}

// Trapezoid wall: connects (x1, near_top, near_bot) with (x2, far_top, far_bot).
void draw_side_wall(SDL_Renderer* r, int near_x, int near_top, int near_bot,
                    int far_x, int far_top, int far_bot) {
    SDL_RenderDrawLine(r, near_x, near_top, far_x, far_top);
    SDL_RenderDrawLine(r, near_x, near_bot, far_x, far_bot);
    SDL_RenderDrawLine(r, near_x, near_top, near_x, near_bot);
    SDL_RenderDrawLine(r, far_x, far_top, far_x, far_bot);
}

}  // namespace

core::Wall front_wall(const core::MazeLevel& m, int x, int y, Facing f) noexcept {
    return get_wall(m, x, y, f, 0);
}

void step_forward(Camera& cam) noexcept {
    cam.x += kDx[static_cast<int>(cam.facing)];
    cam.y += kDy[static_cast<int>(cam.facing)];
}
void step_back(Camera& cam) noexcept {
    cam.x -= kDx[static_cast<int>(cam.facing)];
    cam.y -= kDy[static_cast<int>(cam.facing)];
}
void turn_left(Camera& cam) noexcept {
    cam.facing = static_cast<Facing>((static_cast<int>(cam.facing) + 3) % 4);
}
void turn_right(Camera& cam) noexcept {
    cam.facing = static_cast<Facing>((static_cast<int>(cam.facing) + 1) % 4);
}

void draw_maze_view(SDL_Renderer* r, const core::MazeLevel& level,
                    const Camera& cam, SDL_Rect viewport) {
    const MazePalette palette = current_maze_palette();

    // Ceiling fill (top half) + floor fill (bottom half) — gives the dungeon
    // an actual horizon line so theme swaps read at a glance.
    {
        SDL_Rect ceiling{viewport.x, viewport.y, viewport.w, viewport.h / 2};
        set_color(r, palette.ceiling);
        SDL_RenderFillRect(r, &ceiling);
        SDL_Rect floor{viewport.x, viewport.y + viewport.h / 2,
                       viewport.w, viewport.h - viewport.h / 2};
        set_color(r, palette.floor);
        SDL_RenderFillRect(r, &floor);
    }

    // Outer frame
    set_color(r, palette.frame);
    draw_rect(r, viewport.x, viewport.y,
              viewport.x + viewport.w - 1, viewport.y + viewport.h - 1);

    const int cx = viewport.x + viewport.w / 2;
    const int cy = viewport.y + viewport.h / 2;

    // Concentric "depth" rectangles — 4 cells visible into the distance.
    constexpr int kDepth = 4;
    std::array<float, kDepth + 1> shrink{};
    shrink[0] = 1.0f;
    float ratio = 0.55f;
    for (int i = 1; i <= kDepth; ++i) shrink[i] = shrink[i - 1] * ratio;

    auto rect_at_depth = [&](int d) {
        float s = shrink[d];
        int hw = static_cast<int>((viewport.w / 2) * s);
        int hh = static_cast<int>((viewport.h / 2) * s);
        return SDL_Rect{cx - hw, cy - hh, hw * 2, hh * 2};
    };

    // Step forward virtually for each depth, draw walls.
    int wx = cam.x;
    int wy = cam.y;
    int dx = kDx[static_cast<int>(cam.facing)];
    int dy = kDy[static_cast<int>(cam.facing)];

    for (int depth = 0; depth < kDepth; ++depth) {
        SDL_Rect near_r = rect_at_depth(depth);
        SDL_Rect far_r  = rect_at_depth(depth + 1);

        // Left wall
        core::Wall lw = get_wall(level, wx, wy, cam.facing, 2);
        if (lw == core::Wall::Wall) {
            set_color(r, palette.wall);
            draw_side_wall(r, near_r.x, near_r.y, near_r.y + near_r.h,
                              far_r.x,  far_r.y,  far_r.y + far_r.h);
        } else if (lw == core::Wall::Door) {
            set_color(r, palette.door);
            draw_side_wall(r, near_r.x, near_r.y, near_r.y + near_r.h,
                              far_r.x,  far_r.y,  far_r.y + far_r.h);
        }

        // Right wall
        core::Wall rw = get_wall(level, wx, wy, cam.facing, 1);
        if (rw == core::Wall::Wall) {
            set_color(r, palette.wall);
            int nrx = near_r.x + near_r.w;
            int frx = far_r.x + far_r.w;
            draw_side_wall(r, nrx, near_r.y, near_r.y + near_r.h,
                              frx, far_r.y,  far_r.y + far_r.h);
        } else if (rw == core::Wall::Door) {
            set_color(r, palette.door);
            int nrx = near_r.x + near_r.w;
            int frx = far_r.x + far_r.w;
            draw_side_wall(r, nrx, near_r.y, near_r.y + near_r.h,
                              frx, far_r.y,  far_r.y + far_r.h);
        }

        // Front wall — if present, draw the far rectangle and stop.
        core::Wall fw = get_wall(level, wx, wy, cam.facing, 0);
        if (fw == core::Wall::Wall) {
            set_color(r, palette.far_wall);
            draw_rect(r, far_r.x, far_r.y,
                      far_r.x + far_r.w, far_r.y + far_r.h);
            break;
        } else if (fw == core::Wall::Door) {
            set_color(r, palette.door);
            int dw = far_r.w / 3;
            int dh = far_r.h * 2 / 3;
            int dx_l = far_r.x + (far_r.w - dw) / 2;
            int dy_t = far_r.y + far_r.h - dh;
            draw_rect(r, dx_l, dy_t, dx_l + dw, dy_t + dh);
        }

        // step forward
        wx += dx;
        wy += dy;
        if (wx < 0 || wx >= core::MazeLevel::kSize ||
            wy < 0 || wy >= core::MazeLevel::kSize) break;
    }
}

}  // namespace wiz::render
