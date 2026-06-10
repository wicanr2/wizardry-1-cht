#pragma once

#include "core/maze.h"
#include "game/state.h"

namespace wiz::game {

// Look up the SquareFeature at the party's current cell. Returns
// SquareFeature::None if the cell is empty or out of bounds.
core::SquareFeature feature_at_party(const State& state);

// Apply a SquareFeature effect to the party state. Handles:
//   Pit         — 1d8 damage to each living member
//   Spinner     — random new facing
//   Teleporter  — random reachable cell on the same level
//   Chute       — (placeholder: cosmetic message; multi-level not wired)
// Returns true if anything actually happened (caller may want to redraw).
bool apply_trap(State& state, core::SquareFeature f);

// Sprinkle a few demo traps on the lazy-loaded demo maze so the player
// can see the system in action. Idempotent: safe to call twice.
void seed_demo_traps(core::MazeLevel& m);

}  // namespace wiz::game
