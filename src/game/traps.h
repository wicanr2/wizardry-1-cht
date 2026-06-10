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

// Generate a procedurally-distinct floor (kSize x kSize). Seeded by
// floor number so each level looks different. Adds a few walls, traps
// and a stairs-down feature.
void build_floor(core::MazeLevel& m, int level_number);

// Save state.maze back to the backing store and swap in `new_level`.
// Places the party at the requested cell (or a default spawn if -1, -1).
void switch_floor(State& state, int new_level, int spawn_x = -1, int spawn_y = -1);

}  // namespace wiz::game
