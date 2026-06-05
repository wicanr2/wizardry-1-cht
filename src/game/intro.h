#pragma once

#include <SDL.h>

#include "game/state.h"
#include "render/ui.h"

namespace wiz::game {

// First-run intro: 5-step new-player guide shown before EdgeOfTown on first
// boot (or anytime via F2). User presses any key to advance, ESC to skip.
// Returns true while the intro is still active.
bool intro_active();
void start_intro();
bool intro_tick(State& state, const SDL_Event* event, const render::UI& ui);

}  // namespace wiz::game
