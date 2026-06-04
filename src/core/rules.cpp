#include "core/rules.h"

#include "core/rng.h"

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

}  // namespace wiz::core
