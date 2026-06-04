#include "render/auto_map.h"

#include <algorithm>

namespace wiz::render {

namespace {

constexpr int kDx[4] = {0, 1, 0, -1};
constexpr int kDy[4] = {-1, 0, 1, 0};

}  // namespace

void reveal_from(core::MazeLevel& m, const Camera& cam) noexcept {
    if (cam.x < 0 || cam.x >= core::MazeLevel::kSize) return;
    if (cam.y < 0 || cam.y >= core::MazeLevel::kSize) return;

    m.visited[cam.y][cam.x] = true;

    // Look up to 4 cells forward, stopping at the first wall.
    int x = cam.x, y = cam.y;
    int d = static_cast<int>(cam.facing);
    for (int step = 0; step < 4; ++step) {
        core::Wall front;
        switch (d) {
            case 0: front = m.north[y][x]; break;
            case 1: front = m.east[y][x]; break;
            case 2: front = m.south[y][x]; break;
            case 3: front = m.west[y][x]; break;
            default: front = core::Wall::Wall;
        }
        if (front == core::Wall::Wall) break;
        x += kDx[d];
        y += kDy[d];
        if (x < 0 || x >= core::MazeLevel::kSize ||
            y < 0 || y >= core::MazeLevel::kSize) break;
        m.visited[y][x] = true;
        if (front == core::Wall::Door || front == core::Wall::HiddenDoor) break;
    }
}

void draw_auto_map(SDL_Renderer* r, const core::MazeLevel& m,
                   const Camera& cam, SDL_Rect rect, const Theme& theme) {
    SDL_SetRenderDrawColor(r, theme.bg.r, theme.bg.g, theme.bg.b, 255);
    SDL_RenderFillRect(r, &rect);

    constexpr int N = core::MazeLevel::kSize;
    int cell_w = (rect.w - 8) / N;
    int cell_h = (rect.h - 8) / N;
    int cell = std::min(cell_w, cell_h);
    if (cell < 4) cell = 4;
    int total = cell * N;
    int ox = rect.x + (rect.w - total) / 2;
    int oy = rect.y + (rect.h - total) / 2;

    // Background grid (faint)
    SDL_SetRenderDrawColor(r, 30, 32, 40, 255);
    for (int i = 0; i <= N; ++i) {
        SDL_RenderDrawLine(r, ox, oy + i * cell, ox + total, oy + i * cell);
        SDL_RenderDrawLine(r, ox + i * cell, oy, ox + i * cell, oy + total);
    }

    // Visited cells (fill)
    SDL_SetRenderDrawColor(r, 50, 70, 90, 255);
    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            if (m.visited[y][x]) {
                SDL_Rect c{ox + x * cell + 1, oy + y * cell + 1,
                           cell - 1, cell - 1};
                SDL_RenderFillRect(r, &c);
            }
        }
    }

    // Walls of visited cells
    auto draw_wall_line = [&](int x1, int y1, int x2, int y2, core::Wall w) {
        if (w == core::Wall::Wall) {
            SDL_SetRenderDrawColor(r, theme.text.r, theme.text.g, theme.text.b, 255);
            SDL_RenderDrawLine(r, x1, y1, x2, y2);
        } else if (w == core::Wall::Door) {
            SDL_SetRenderDrawColor(r, theme.accent.r, theme.accent.g, theme.accent.b, 255);
            SDL_RenderDrawLine(r, x1, y1, x2, y2);
        }
    };

    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            if (!m.visited[y][x]) continue;
            int px = ox + x * cell;
            int py = oy + y * cell;
            int pw = cell;
            draw_wall_line(px, py, px + pw, py, m.north[y][x]);
            draw_wall_line(px, py + pw, px + pw, py + pw, m.south[y][x]);
            draw_wall_line(px, py, px, py + pw, m.west[y][x]);
            draw_wall_line(px + pw, py, px + pw, py + pw, m.east[y][x]);
        }
    }

    // Current camera position — filled diamond with facing arrow
    SDL_SetRenderDrawColor(r, 230, 180, 60, 255);
    int px = ox + cam.x * cell + cell / 2;
    int py = oy + cam.y * cell + cell / 2;
    SDL_Rect dot{px - cell / 4, py - cell / 4, cell / 2, cell / 2};
    SDL_RenderFillRect(r, &dot);

    // Facing indicator
    int fx = px, fy = py;
    switch (cam.facing) {
        case Facing::North: fy = py - cell;     break;
        case Facing::East:  fx = px + cell;     break;
        case Facing::South: fy = py + cell;     break;
        case Facing::West:  fx = px - cell;     break;
    }
    SDL_RenderDrawLine(r, px, py, fx, fy);

    // Frame border
    SDL_SetRenderDrawColor(r, theme.border.r, theme.border.g, theme.border.b, 255);
    SDL_Rect frame{rect.x, rect.y, rect.w, rect.h};
    SDL_RenderDrawRect(r, &frame);
}

}  // namespace wiz::render
