#pragma once

#include <string>

#include "game/state.h"

namespace wiz::save {

// Whole-game snapshot: roster + party composition + maze position.
// JSON v2; backwards-compatible with v1 roster-only files.

bool save_game(const game::State& state, const std::string& path);
bool load_game(game::State& state, const std::string& path);

}  // namespace wiz::save
