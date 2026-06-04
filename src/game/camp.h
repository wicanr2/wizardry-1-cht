#pragma once

#include <SDL.h>

#include "game/state.h"
#include "render/ui.h"

namespace wiz::game {

// Camp — pause-screen inside the maze. Save game, inspect characters,
// rearrange party, return to maze, or quit to title.
bool camp_tick(State& state, const SDL_Event* event, const render::UI& ui);

// Path of the default save slot. Per-platform appropriate.
std::string default_save_path();

}  // namespace wiz::game
