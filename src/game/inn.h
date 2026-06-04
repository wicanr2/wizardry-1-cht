#pragma once

#include <SDL.h>

#include "game/state.h"
#include "render/ui.h"

namespace wiz::game {

// Adventurer's Inn — pay to rest, recover HP (and level up if XP suffices).
bool inn_tick(State& state, const SDL_Event* event, const render::UI& ui);

// Cant's Temple — heal status afflictions, resurrect dead/ashes.
bool temple_tick(State& state, const SDL_Event* event, const render::UI& ui);

}  // namespace wiz::game
