#pragma once

#include <SDL.h>

#include "game/state.h"
#include "render/ui.h"

namespace wiz::game {

// Each screen handles input and rendering for one Scene.
// Returns true while the game should keep running; false to quit.
bool tick(State& state, const SDL_Event* event, const render::UI& ui);

}  // namespace wiz::game
