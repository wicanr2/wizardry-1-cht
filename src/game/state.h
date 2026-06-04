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
    Quit,
};

struct Party {
    std::array<int, 6> roster_index{ -1, -1, -1, -1, -1, -1 };  // indices into Roster
    int count = 0;
};

struct State {
    save::Roster roster;
    Party party;
    Scene scene = Scene::Title;
    Scene prev_scene = Scene::Title;
    std::vector<std::string> message_log;
    std::string status_hint;
    bool dirty = true;

    core::MazeLevel maze;
    render::Camera camera;
    bool maze_loaded = false;

    core::CombatState combat;

    // Forward-declared; full RollerState defined in game/roller.h. Stored as
    // pointer-like to avoid pulling in the roller header for every consumer.
    std::shared_ptr<void> roller;

    void push_message(std::string m);
    void change_scene(Scene s);
};

}  // namespace wiz::game
