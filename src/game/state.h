#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "core/character.h"
#include "core/combat.h"
#include "core/maze.h"
#include "render/maze_view.h"
#include "save/roster.h"

namespace wiz::game {

enum class Scene {
    Title,
    EdgeOfTown,
    Castle,
    Tavern,
    Shop,
    Temple,
    Inn,
    TrainingGrounds,
    Camp,
    Maze,
    Combat,
    Ending,
    Quit,
};

struct Party {
    std::array<int, 6> roster_index{ -1, -1, -1, -1, -1, -1 };  // indices into Roster
    int count = 0;
};

// A character who died in the maze; their body stays where they fell
// until a new party walks onto that cell and picks them up.
struct DeadBody {
    int roster_idx = -1;   // index into Roster::chars
    int level = 1;
    int x = 0;
    int y = 0;
};

constexpr int kMaxFloors = 10;

struct State {
    save::Roster roster;
    Party party;
    Scene scene = Scene::Title;
    Scene prev_scene = Scene::Title;
    std::vector<std::string> message_log;
    std::string status_hint;
    bool dirty = true;

    // `maze` is the live view of the current floor — every call site reads
    // through this. On Chute / Stairs we copy it back to `mazes[level-1]`
    // and swap in the destination floor.
    core::MazeLevel maze;
    std::array<core::MazeLevel, kMaxFloors> mazes{};
    std::array<bool, kMaxFloors> floor_built{};
    render::Camera camera;
    bool maze_loaded = false;

    // Bodies waiting to be retrieved.
    std::vector<DeadBody> dead_bodies;

    core::CombatState combat;

    // Anti-magic zone effect set by Fizzle square; cleared when the party
    // steps onto a non-Fizzle cell. cast_camp_spell + combat cast_spell
    // check this and refuse to fire.
    bool anti_magic_here = false;

    // Encounter feature (SquareFeature::Encounter) sets this; the maze
    // step handler in screens.cpp reads it next frame to spawn combat.
    bool pending_force_encounter = false;

    // LATUMAPIC cast in camp persists into the next combat — begin_combat()
    // consumes this and pre-identifies every monster group.
    bool latumapic_next_combat = false;

    // F5 / M toggles the auto-map overlay in the maze view. Persisted in
    // memory only — the user re-toggles per session.
    bool show_auto_map = true;

    // Camp light state set by MILWA (short) / LOMILWA (long). Affects the
    // maze_view rendering tint inside dark zones — without light, walls
    // beyond 1 cell are nearly invisible.
    int light_steps_left = 0;  // 0 = no light; MILWA sets ~20, LOMILWA ~200

    // Set when the party defeats Werdna on B10F — screens.cpp routes the
    // post-combat continue into Scene::Ending instead of Scene::Maze.
    bool werdna_defeated = false;

    // Forward-declared; full RollerState defined in game/roller.h. Stored as
    // pointer-like to avoid pulling in the roller header for every consumer.
    std::shared_ptr<void> roller;

    void push_message(std::string m);
    void change_scene(Scene s);
};

}  // namespace wiz::game
