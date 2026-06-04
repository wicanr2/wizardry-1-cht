#pragma once

#include <SDL.h>

#include <string>

#include "core/character.h"
#include "game/state.h"
#include "render/ui.h"

namespace wiz::game {

enum class RollerStep {
    Name,
    Race,
    Alignment,
    AttributeRoll,
    DistributeBonus,
    PickClass,
    Confirm,
    Done,
};

struct RollerState {
    RollerStep step = RollerStep::Name;
    std::string name_buf;
    core::Race race = core::Race::Human;
    core::Alignment alignment = core::Alignment::Good;
    core::Attributes attr{};
    int bonus_pts = 0;
    int focused_attr = 0;  // 0..5 cursor in distribute phase
    core::Klass klass = core::Klass::Fighter;
    int cursor = 0;        // generic list cursor
};

// Handle one input/render tick for the Roller; returns true while still active.
// When step reaches Done, caller should write `out` into roster + change scene.
bool roller_tick(RollerState& roll, State& state, const SDL_Event* event,
                 const render::UI& ui, core::Character& out);

}  // namespace wiz::game
