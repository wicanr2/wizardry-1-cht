#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "core/character.h"
#include "core/monster.h"

namespace wiz::core {

struct CombatGroup {
    Monster prototype;
    std::int16_t alive_count = 0;
    std::int16_t total_count = 0;
    bool identified = false;
    std::int16_t hp_total = 0;  // pooled HP across the group
};

enum class CombatPhase {
    Engage,            // intro: "X MONSTERS APPEAR!"
    PartyAction,       // pick attack / spell / run / parry per character
    PickSpell,         // sub-menu for spell selection
    PickTarget,        // sub-menu for fight/spell target
    Resolve,           // resolve player actions then monster actions
    End,               // victory / defeat / fled
};

enum class CombatOutcome { Ongoing, Victory, Defeat, Fled };

struct PlayerAction {
    enum Kind : std::uint8_t { Fight, Spell, Parry, Run, UseItem } kind = Fight;
    int target_group = 0;
    std::string spell_name;
};

struct CombatState {
    std::vector<CombatGroup> groups;
    std::array<PlayerAction, 6> actions{};
    int active_party_member = 0;
    CombatPhase phase = CombatPhase::Engage;
    CombatOutcome outcome = CombatOutcome::Ongoing;
    long_t xp_award = 0;
    long_t gold_award = 0;
    std::vector<std::string> log;
};

// Initiates a combat encounter — drops you into PartyAction phase.
void begin_combat(CombatState& s, std::vector<CombatGroup> groups);

// Resolves one full round once all 6 actions are queued.
void resolve_round(CombatState& s, std::array<Character, 6>& party);

// Player action selection helpers
bool set_action(CombatState& s, int member_idx, PlayerAction a);

// Render-side accessors
const char* phase_label(CombatPhase p) noexcept;

}  // namespace wiz::core
