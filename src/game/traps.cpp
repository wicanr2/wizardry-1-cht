#include "game/traps.h"

#include <cstdio>

#include "core/rng.h"
#include "data/maze_db.h"

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

// Anti-magic / "fizzle" zone — a single-cell dampener that blocks every
// spell cast (combat or camp) while the party stands on it. Reset when
// the party steps off (handled by the maze step handler clearing it).
static void anti_magic(State& state) {
    state.anti_magic_here = true;
    state.push_message("** 你感到法力被空氣吸走 — 此處反魔法。");
}

// Message square — flavour text read once per visit. Multi-floor maze data
// stores the message index in aux0 / aux1; in the absence of a per-level
// catalogue we hash (level, idx) into a small set of plausible Wizardry I
// vignettes from the original signs (per tk421 walk-through).
static void message_square(State& state) {
    static const char* kMessages[] = {
        "牆上刻著：「BEWARE WERDNA」",
        "你看見舊冒險者的骨骸蜷縮在角落。",
        "石板寫著：「TRESPASSER — TURN BACK」",
        "牆面隱約浮現 Trebor 的徽記。",
        "黴味中飄來遠處的鐘聲，無人能說來自何方。",
        "地上散落幾枚生鏽的硬幣。",
    };
    int n = static_cast<int>(sizeof(kMessages) / sizeof(kMessages[0]));
    int idx = (state.camera.level * 17 + state.camera.x * 3 + state.camera.y * 7) % n;
    state.push_message(std::string("** 留言：") + kMessages[idx]);
}

// Elevator — original W1 elevator at B4 between B4 and B9. Simple stop-by-
// stop selector: each press of the trap cycles up to the next floor in the
// range (4..9). We just take the user to the next floor up in that range.
static void elevator(State& state) {
    int cur = state.camera.level;
    int next = cur + 1;
    if (next < 4) next = 4;
    if (next > 9) next = 4;  // wrap back to top
    if (next == cur) {
        state.push_message("** 電梯軋軋作響，但門開又關 — 沒移動。**");
        return;
    }
    switch_floor(state, next, -1, -1);
    char buf[120];
    std::snprintf(buf, sizeof(buf),
                  "** 電梯升降！到達 B%dF。**", next);
    state.push_message(buf);
}

// Forced encounter — bumps you into a fight without rolling the per-step
// probability. The maze step handler also reads `fights[y][x]` for the
// classic always-encounter cell; this is the feature-slot variant.
static void force_encounter(State& state) {
    state.push_message("** 你踏入埋伏 — 戰鬥不可避免！");
    state.pending_force_encounter = true;
}

static void chute(State& state) {
    int cur = state.camera.level;
    if (cur >= kMaxFloors) {
        state.push_message("** 滑梯! 但這已是最深層,卡在原地。**");
        return;
    }
    // Random cell on the destination floor.
    auto& rng = core::global_rng();
    int nx = rng.range(0, core::MazeLevel::kSize - 1);
    int ny = rng.range(0, core::MazeLevel::kSize - 1);
    switch_floor(state, cur + 1, nx, ny);
    char buf[120];
    std::snprintf(buf, sizeof(buf),
                  "** 滑梯! 跌落 B%dF 的暗處 (%d, %d)。**",
                  cur + 1, nx, ny);
    state.push_message(buf);
}

bool apply_trap(State& state, SquareFeature f) {
    switch (f) {
        case SquareFeature::Pit:        pit_damage(state);     return true;
        case SquareFeature::Spinner:    spinner(state);        return true;
        case SquareFeature::Teleporter: teleporter(state);     return true;
        case SquareFeature::Chute:      chute(state);          return true;
        case SquareFeature::Fizzle:     anti_magic(state);     return true;
        case SquareFeature::Message:    message_square(state); return true;
        case SquareFeature::Elevator:   elevator(state);       return true;
        case SquareFeature::Encounter:  force_encounter(state);return true;
        default: return false;
    }
}

void build_floor(core::MazeLevel& m, int level_number) {
    using core::Wall;
    using core::MazeLevel;
    // Prefer the transcribed 1981 layout from assets/data/wiz1_mazes.json
    // when available; fall back to the procedural placeholder.
    const auto& bundled = data::mazes();
    if (level_number >= 1 &&
        static_cast<std::size_t>(level_number) <= bundled.size()) {
        m = bundled[level_number - 1];
        return;
    }
    m = MazeLevel{};
    m.level_number = static_cast<std::uint8_t>(level_number);
    // All cells open inside, walls on the outer rim.
    for (int y = 0; y < MazeLevel::kSize; ++y) {
        for (int x = 0; x < MazeLevel::kSize; ++x) {
            m.west[y][x]  = (x == 0) ? Wall::Wall : Wall::Open;
            m.east[y][x]  = (x == MazeLevel::kSize - 1) ? Wall::Wall : Wall::Open;
            m.north[y][x] = (y == 0) ? Wall::Wall : Wall::Open;
            m.south[y][x] = (y == MazeLevel::kSize - 1) ? Wall::Wall : Wall::Open;
        }
    }
    // Carve a few level-dependent interior walls so each floor differs.
    // Pattern shifts with the level number; deeper floors get more walls.
    int seed = level_number * 7;
    int walls = level_number + 2;
    for (int i = 0; i < walls; ++i) {
        int x = (seed + i * 3) % (MazeLevel::kSize - 1);
        int y = (seed * 2 + i * 5) % (MazeLevel::kSize - 1);
        m.east[y][x] = Wall::Wall;
        m.west[y][x + 1] = Wall::Wall;
    }
    // Traps: slot 1=Pit, 2=Spinner, 3=Teleporter, 4=Chute, 5=Stairs(down).
    m.sqre_type[1] = core::SquareFeature::Pit;
    m.sqre_type[2] = core::SquareFeature::Spinner;
    m.sqre_type[3] = core::SquareFeature::Teleporter;
    m.sqre_type[4] = core::SquareFeature::Chute;
    m.sqre_type[5] = core::SquareFeature::Stairs;
    // Sprinkle one of each on this floor (positions vary by level).
    int base = (level_number * 11) % (MazeLevel::kSize - 2);
    m.sqr_extra[base + 0][1] = 1;             // pit near west wall
    m.sqr_extra[1][base + 1] = 2;             // spinner near north wall
    m.sqr_extra[base + 2][base + 2] = 3;      // teleporter diagonal
    m.sqr_extra[base + 3][2] = 4;             // chute
    if (level_number < kMaxFloors) {
        m.sqr_extra[MazeLevel::kSize - 2][MazeLevel::kSize - 2] = 5;  // stairs down
    }
    // Encounter probability bytes (0-255). Original Wizardry I scales
    // monster-spawn rate with floor depth — roughly 30 + 7*(level-1).
    int p = 30 + 7 * (level_number - 1);
    if (p > 110) p = 110;
    m.enmy_calc[0] = static_cast<std::uint8_t>(p);
    m.enmy_calc[1] = 75;   // 75% normal table / 25% special table
    m.enmy_calc[2] = static_cast<std::uint8_t>(level_number);
}

void switch_floor(State& state, int new_level, int spawn_x, int spawn_y) {
    if (new_level < 1) new_level = 1;
    if (new_level > kMaxFloors) new_level = kMaxFloors;
    int old_level = state.camera.level;
    // Save current floor back to backing store.
    if (old_level >= 1 && old_level <= kMaxFloors) {
        state.mazes[old_level - 1] = state.maze;
        state.floor_built[old_level - 1] = true;
    }
    // Load destination (build lazily on first visit).
    if (!state.floor_built[new_level - 1]) {
        build_floor(state.mazes[new_level - 1], new_level);
        state.floor_built[new_level - 1] = true;
    }
    state.maze = state.mazes[new_level - 1];
    state.camera.level = new_level;
    if (spawn_x >= 0 && spawn_y >= 0) {
        state.camera.x = spawn_x;
        state.camera.y = spawn_y;
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
