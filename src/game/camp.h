#pragma once

#include <SDL.h>

#include "game/state.h"
#include "render/ui.h"

namespace wiz::game {

// Camp — pause-screen inside the maze. Save game, inspect characters,
// rearrange party, cast utility spells, return to maze, or quit to title.
bool camp_tick(State& state, const SDL_Event* event, const render::UI& ui);

// Path of save slot N (1..kNumSlots). Per-platform appropriate.
constexpr int kNumSlots = 5;
std::string save_path_for_slot(int slot);  // slot in 1..kNumSlots
std::string default_save_path();           // returns save_path_for_slot(1)

// Cast a utility spell (DIOS/DIAL/DIALMA/MADI/MILWA/CALFO/...) on the party,
// while we're in camp. Returns the result message (always non-empty).
std::string cast_camp_spell(State& state, std::string_view spell_name,
                            int target_member);

}  // namespace wiz::game
