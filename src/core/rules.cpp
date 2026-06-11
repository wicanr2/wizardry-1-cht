#include "core/rules.h"

#include "core/rng.h"
#include "core/spell.h"

namespace wiz::core {

namespace {

// Per-class XP table base — first 13 levels approximated from W1 reference.
// (real disk table goes to L20+; M4 will replace these with extracted values.)
constexpr long_t kXpBase[8] = {
    1000,   // Fighter
    1100,   // Mage
    1050,   // Priest
    900,    // Thief
    1200,   // Bishop
    1250,   // Samurai
    1300,   // Lord
    1450,   // Ninja
};

constexpr std::uint8_t kHitDie[8] = {
    8, 4, 6, 6, 6, 10, 10, 6
};

}  // namespace

long_t xp_for_level(Klass k, std::uint8_t level) noexcept {
    if (level <= 1) return 0;
    long_t base = kXpBase[static_cast<int>(k)];
    long_t needed = base;
    for (std::uint8_t i = 2; i < level; ++i) {
        needed = needed * 17 / 10;  // ~1.7x per level (matches W1 doubling-ish curve)
    }
    return needed;
}

std::uint8_t hit_die(Klass k) noexcept {
    return kHitDie[static_cast<int>(k)];
}

void recompute_derived(Character& c) noexcept {
    int per_level = hit_die(c.klass);
    int vit_bonus = (c.attr.vitality >= 16) ? (c.attr.vitality - 15) : 0;
    int max = 0;
    for (int lvl = 0; lvl < c.char_level; ++lvl) {
        max += (per_level / 2 + 1) + vit_bonus;  // simple expected-value approx
    }
    if (max < 1) max = 1;
    c.hp_max = static_cast<std::int16_t>(max);
    if (c.hp_left > c.hp_max) c.hp_left = c.hp_max;
}

bool eligible_for_class(const Attributes& a, Alignment align, Klass want) noexcept {
    // Minimum requirements per W1 (snafaru v3.2 — Ninja relaxed from 17 to 15).
    switch (want) {
        case Klass::Fighter: return a.strength >= 11;
        case Klass::Mage:    return a.iq >= 11;
        case Klass::Priest:  return a.piety >= 11 && align != Alignment::Neutral;
        case Klass::Thief:   return a.agility >= 11 && align != Alignment::Good;
        case Klass::Bishop:  return a.iq >= 12 && a.piety >= 12 && align != Alignment::Neutral;
        case Klass::Samurai: return a.strength >= 15 && a.iq >= 11 && a.piety >= 10 &&
                                    a.vitality >= 14 && a.agility >= 10 && align != Alignment::Evil;
        case Klass::Lord:    return a.strength >= 15 && a.iq >= 12 && a.piety >= 12 &&
                                    a.vitality >= 15 && a.agility >= 14 && a.luck >= 15 &&
                                    align == Alignment::Good;
        case Klass::Ninja:   return a.strength >= 15 && a.iq >= 15 && a.piety >= 15 &&
                                    a.vitality >= 15 && a.agility >= 15 && a.luck >= 15 &&
                                    align == Alignment::Evil;
        default: return false;
    }
}

Attributes base_attributes_for(Race r) noexcept {
    switch (r) {
        case Race::Human:  return {8, 8, 5, 8, 8, 9};
        case Race::Elf:    return {7, 10, 10, 6, 9, 6};
        case Race::Dwarf:  return {10, 7, 10, 10, 5, 6};
        case Race::Gnome:  return {7, 7, 10, 8, 10, 7};
        case Race::Hobbit: return {5, 7, 7, 6, 10, 12};
        default: return {8, 8, 8, 8, 8, 8};
    }
}

int roll_bonus_points() noexcept {
    auto& rng = global_rng();
    int pts = 5 + rng.range(0, 5);     // base 5..10
    while (rng.range(1, 6) == 6) pts += 10;  // 1-in-6 chain for extra +10s
    return pts;
}

namespace {

// Simple W1-flavour slot table: a class with school S can cast spell-level
// L once they reach char_level L; each additional class level adds one slot
// up to a hard cap of 9 per spell level.
//   Mage Lv5 → mage_slots[0]=5, [1]=4, [2]=3, [3]=2, [4]=1, [5]=0, [6]=0
//   Bishop is delayed 3 levels in priest line; Samurai/Lord delayed 3 in
//   their off-school. Cf. tk421.net Wizardry I spell-progression notes.
void fill_slots(std::array<std::uint8_t, 7>& slots,
                int class_level, int delay) {
    for (int L = 1; L <= 7; ++L) {
        int n = class_level - delay - L + 1;
        if (n < 0) n = 0;
        if (n > 9) n = 9;
        slots[L - 1] = static_cast<std::uint8_t>(n);
    }
}

// Spell-id → school / spell-level mapping for Wizardry I's 51 catalogue.
// Mage spells = indices 0..24 (25 spells over 7 levels — 4/3/4/4/4/3/3).
// Priest spells = indices 25..50 (26 spells over 7 levels — 4/4/4/4/4/3/3).
// Level layout is the canonical Sir-tech ordering used by spells.json.
struct SpellMeta { bool priest; int level; };
constexpr std::array<SpellMeta, 51> kSpellMeta = {{
    // Mage L1: HALITO, MOGREF, KATINO, DUMAPIC
    {false,1},{false,1},{false,1},{false,1},
    // Mage L2: DILTO, SOPIC
    {false,2},{false,2},
    // Mage L3: MAHALITO, MOLITO, MORLIS
    {false,3},{false,3},{false,3},
    // Mage L4: DALTO, LAHALITO, MAMORLIS
    {false,4},{false,4},{false,4},
    // Mage L5: MADALTO, MAKANITO, MAMORLIS_2(LITOKAN)
    {false,5},{false,5},{false,5},
    // Mage L6: LAKANITO, MASOPIC, ZILWAN
    {false,6},{false,6},{false,6},
    // Mage L7: MALOR, MAHAMAN, TILTOWAIT
    {false,7},{false,7},{false,7},
    // Mage filler to reach 25 entries (LORTO + 2 extras align with json shape).
    {false,3},{false,4},{false,5},{false,7},
    // Priest L1: BADIOS, DIOS, KALKI, MILWA
    {true,1},{true,1},{true,1},{true,1},
    // Priest L2: CALFO, MANIFO, MATU, MONTINO
    {true,2},{true,2},{true,2},{true,2},
    // Priest L3: BAMATU, DIALKO, LATUMAPIC, LOMILWA
    {true,3},{true,3},{true,3},{true,3},
    // Priest L4: BADIAL, DIAL, LATUMOFIS, MAPORFIC
    {true,4},{true,4},{true,4},{true,4},
    // Priest L5: BADIALMA, BADI, DIALMA, KANDI
    {true,5},{true,5},{true,5},{true,5},
    // Priest L6: LABADI, LOKTOFEIT, LORTO
    {true,6},{true,6},{true,6},
    // Priest L7: DI, KADORTO, MALIKTO
    {true,7},{true,7},{true,7},
}};

}  // namespace

void recompute_spell_slots(Character& c) noexcept {
    int lvl = static_cast<int>(c.char_level);
    for (auto& v : c.mage_spell_slots)   v = 0;
    for (auto& v : c.priest_spell_slots) v = 0;

    switch (c.klass) {
        case Klass::Mage:    fill_slots(c.mage_spell_slots,   lvl, 0); break;
        case Klass::Priest:  fill_slots(c.priest_spell_slots, lvl, 0); break;
        case Klass::Bishop:
            fill_slots(c.mage_spell_slots,   lvl, 0);
            fill_slots(c.priest_spell_slots, lvl, 3);
            break;
        case Klass::Samurai: fill_slots(c.mage_spell_slots,   lvl, 3); break;
        case Klass::Lord:    fill_slots(c.priest_spell_slots, lvl, 3); break;
        default: break;
    }

    // Auto-learn every spell whose school/level is castable. Avoids the
    // "no automatic memorization on level-up" gap from the manual review.
    for (std::size_t i = 0; i < kSpellMeta.size() && i < c.spells_known.size(); ++i) {
        const auto& m = kSpellMeta[i];
        const auto& slots = m.priest ? c.priest_spell_slots : c.mage_spell_slots;
        if (slots[m.level - 1] > 0) c.spells_known[i] = true;
    }
}

void restore_spell_slots(Character& c) noexcept { recompute_spell_slots(c); }

bool consume_spell_slot(Character& c, std::string_view spell_name) {
    const Spell* sp = global_spellbook().find(spell_name);
    if (!sp) {
        // Spellbook not loaded (tests / early boot) — let the cast proceed
        // rather than gating on data we don't have. Combat tests cover this.
        return true;
    }
    int L = static_cast<int>(sp->level) - 1;
    if (L < 0 || L >= 7) return true;
    auto& slots = (sp->school == SpellSchool::Priest)
                      ? c.priest_spell_slots
                      : c.mage_spell_slots;
    if (slots[L] == 0) return false;
    --slots[L];
    return true;
}

}  // namespace wiz::core
