#include "game/traps.h"

#include <cstdio>

#include "core/rng.h"

namespace wiz::game {

using core::SquareFeature;

core::SquareFeature feature_at_party(const State& state) {
    int x = state.camera.x;
    int y = state.camera.y;
    if (x < 0 || y < 0 || x >= core::MazeLevel::kSize ||
        y >= core::MazeLevel::kSize) {
        return SquareFeature::None;
    }
    std::uint8_t idx = state.maze.sqr_extra[y][x];
    if (idx >= state.maze.sqre_type.size()) return SquareFeature::None;
    return state.maze.sqre_type[idx];
}

static void pit_damage(State& state) {
    auto& rng = core::global_rng();
    int total = 0;
    int wounded = 0;
    for (int i = 0; i < state.party.count; ++i) {
        int ri = state.party.roster_index[i];
        if (ri < 0) continue;
        auto& c = state.roster.chars[ri];
        if (c.status == core::Status::Dead ||
            c.status == core::Status::Ashes ||
            c.status == core::Status::Lost) continue;
        int dmg = rng.dice(1, 8);
        c.hp_left = static_cast<std::int16_t>(c.hp_left - dmg);
        total += dmg;
        ++wounded;
        if (c.hp_left <= 0) {
            c.status = core::Status::Dead;
            state.push_message(c.name + " 跌死於陷阱中。");
        }
    }
    char buf[160];
    std::snprintf(buf, sizeof(buf),
                  "** 陷阱坑！** 全隊共受 %d 傷害（%d 人受傷）。",
                  total, wounded);
    state.push_message(buf);
}

static void spinner(State& state) {
    auto& rng = core::global_rng();
    int facing = rng.range(0, 3);
    state.camera.facing = static_cast<render::Facing>(facing);
    state.push_message("** 你被旋轉了！迷失方向… **");
}

static void teleporter(State& state) {
    auto& rng = core::global_rng();
    int nx = rng.range(0, core::MazeLevel::kSize - 1);
    int ny = rng.range(0, core::MazeLevel::kSize - 1);
    state.camera.x = nx;
    state.camera.y = ny;
    char buf[80];
    std::snprintf(buf, sizeof(buf),
                  "** 傳送！座標躍至 (%d, %d)。**", nx, ny);
    state.push_message(buf);
}

static void chute(State& state) {
    // Multi-level dungeon not yet wired; for now this just messages.
    state.push_message("** 滑梯！你掉到下一層的暗處… **");
    (void)state;
}

bool apply_trap(State& state, SquareFeature f) {
    switch (f) {
        case SquareFeature::Pit:        pit_damage(state); return true;
        case SquareFeature::Spinner:    spinner(state);    return true;
        case SquareFeature::Teleporter: teleporter(state); return true;
        case SquareFeature::Chute:      chute(state);      return true;
        default: return false;
    }
}

void seed_demo_traps(core::MazeLevel& m) {
    // Slot 0 is implicitly None. Use slots 1..4 for our four trap kinds.
    m.sqre_type[1] = SquareFeature::Pit;
    m.sqre_type[2] = SquareFeature::Spinner;
    m.sqre_type[3] = SquareFeature::Teleporter;
    m.sqre_type[4] = SquareFeature::Chute;

    // Sprinkle a few cells with each kind so the player encounters them
    // during the demo walk. Cells picked to be inside the carved corridor
    // (the demo maze starts at (0, 5) facing north).
    struct Place { int x, y; std::uint8_t kind; };
    constexpr Place kSeeds[] = {
        {0, 3, 1},  // pit
        {1, 1, 2},  // spinner
        {3, 0, 3},  // teleporter
        {2, 4, 4},  // chute
    };
    for (const auto& p : kSeeds) {
        if (p.x < core::MazeLevel::kSize && p.y < core::MazeLevel::kSize) {
            m.sqr_extra[p.y][p.x] = p.kind;
        }
    }
}

}  // namespace wiz::game
