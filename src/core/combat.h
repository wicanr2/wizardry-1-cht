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

    // Per-round status flags toggled by spells:
    std::int8_t ac_mod = 0;    // DILTO/MORLIS adds positive (worse for them)
    bool asleep = false;       // KATINO/MANIFO
    bool silenced = false;     // MONTINO
    bool feared = false;       // MORLIS/MAMORLIS
    bool paralyzed = false;    // MANIFO (priest)
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

// First-round surprise state. Rolled in begin_combat().
//   None         — both sides act normally
//   PartyAhead   — monsters skip their phase on round 1 (party gets free hit)
//   MonstersAhead — party can only Parry/Run on round 1; monsters get free swing
enum class Surprise : std::uint8_t { None = 0, PartyAhead, MonstersAhead };

struct PlayerAction {
    enum Kind : std::uint8_t { Fight, Spell, Parry, Run, UseItem } kind = Fight;
    int target_group = 0;
    std::string spell_name;
};

struct PartyBuff {
    std::int8_t ac_mod = 0;          // negative = better; applied to each member
    bool magic_resist = false;       // MOGATO
    bool combat_only = true;         // false for MAPORFIC (lasts till next combat)
};

struct CombatState {
    std::vector<CombatGroup> groups;
    std::array<PlayerAction, 6> actions{};
    std::array<std::int8_t, 6> per_member_ac{};  // MOGREF/SOPIC/PORFIC stacks
    PartyBuff party_buff{};
    int active_party_member = 0;
    CombatPhase phase = CombatPhase::Engage;
    CombatOutcome outcome = CombatOutcome::Ongoing;
    long_t xp_award = 0;
    long_t gold_award = 0;
    std::vector<std::string> log;

    // First-round bookkeeping for surprise / breath weapon timing.
    Surprise surprise = Surprise::None;
    int round = 0;  // increments at the start of each resolve_round
};

// Initiates a combat encounter — drops you into PartyAction phase.
void begin_combat(CombatState& s, std::vector<CombatGroup> groups);

// Resolves one full round once all 6 actions are queued.
void resolve_round(CombatState& s, std::array<Character, 6>& party);

// Player action selection helpers
bool set_action(CombatState& s, int member_idx, PlayerAction a);

// Cast a spell — public for testing.
void cast_spell(CombatState& s, int caster_idx, Character& caster,
                const PlayerAction& a);

// Render-side accessors
const char* phase_label(CombatPhase p) noexcept;

}  // namespace wiz::core
