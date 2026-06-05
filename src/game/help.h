#pragma once

#include "game/state.h"
#include "render/ui.h"

namespace wiz::game {

// Help overlay — toggled by F1. Shows the key-bindings for the current scene.
// Returns true if the overlay is currently visible (caller should skip its own
// input handling while help is shown).
bool help_active();
void toggle_help();
void draw_help(Scene scene, const render::UI& ui);

}  // namespace wiz::game
