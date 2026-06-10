#pragma once

#include <vector>

#include "core/maze.h"

namespace wiz::data {

// Load the bundled wiz1_mazes.json (transcribed from the original 1981
// PDF maps). Returns a 10-element vector of MazeLevel; on failure
// returns an empty vector and the caller should fall back to a
// procedural builder.
const std::vector<core::MazeLevel>& mazes();

}  // namespace wiz::data
