#pragma once

#include <SDL.h>

#include "game/state.h"
#include "render/ui.h"

namespace wiz::game {

// Gilgamesh's Tavern — add/remove party members from the roster.
bool tavern_tick(State& state, const SDL_Event* event, const render::UI& ui);

}  // namespace wiz::game
