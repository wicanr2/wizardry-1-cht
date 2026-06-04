#pragma once

#include <SDL.h>

#include "game/state.h"
#include "render/ui.h"

namespace wiz::game {

// Boltac's Trading Post — buy / sell / inspect.
// Currently MVP: character-shared purse, browse + buy. Sell / identify / uncurse
// are stubbed.
bool shop_tick(State& state, const SDL_Event* event, const render::UI& ui);

}  // namespace wiz::game
