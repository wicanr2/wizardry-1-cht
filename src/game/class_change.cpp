#include "game/class_change.h"

namespace wiz::game {

using core::Klass;
using core::Alignment;

ClassRequirement requirements_for(Klass k) {
    switch (k) {
        case Klass::Fighter: return {.str = 11};
        case Klass::Mage:    return {.iq  = 11};
        case Klass::Priest:  return {.pie = 11};
        case Klass::Thief:   return {.agi = 11};
        case Klass::Bishop:  return {.iq = 12, .pie = 12};
        case Klass::Samurai: return {.str = 15, .iq = 11, .pie = 10,
                                     .vit = 14, .agi = 10};
        case Klass::Lord:    return {.str = 15, .iq = 12, .pie = 12,
                                     .vit = 15, .agi = 14, .luc = 15};
        case Klass::Ninja:   return {.str = 15, .iq = 15, .pie = 15,
                                     .vit = 15, .agi = 15, .luc = 15};
        default: return {};
    }
}

static bool alignment_allowed(Klass k, Alignment a) {
    if (k == Klass::Priest && a == Alignment::Neutral)   return false;
    if (k == Klass::Samurai && a == Alignment::Evil)     return false;
    if (k == Klass::Lord && a != Alignment::Good)        return false;
    if (k == Klass::Ninja && a != Alignment::Evil)       return false;
    if (k == Klass::Bishop && a == Alignment::Neutral)   return false;
    return true;
}

std::string can_change_to(const core::Character& c, Klass target) {
    if (target == c.klass) return "與目前職業相同";
    if (!alignment_allowed(target, c.alignment)) return "陣營禁止此職業";
    auto r = requirements_for(target);
    if (c.attr.strength < r.str) return "力量不足";
    if (c.attr.iq       < r.iq)  return "智力不足";
    if (c.attr.piety    < r.pie) return "信仰不足";
    if (c.attr.vitality < r.vit) return "活力不足";
    if (c.attr.agility  < r.agi) return "敏捷不足";
    if (c.attr.luck     < r.luc) return "幸運不足";
    return {};
}

std::string_view apply_class_change(core::Character& c, Klass target) {
    c.klass = target;
    c.char_level = 1;
    c.experience = 0;
    c.hp_max = static_cast<std::int16_t>(c.hp_max / 2);
    if (c.hp_max < 1) c.hp_max = 1;
    c.hp_left = c.hp_max;
    // Spells_known are retained (the manual lets reborn classes keep what
    // they once memorised); current spell slots reset until next rest.
    for (auto& v : c.mage_spell_slots)   v = 0;
    for (auto& v : c.priest_spell_slots) v = 0;
    // Retraining costs time.
    c.age = static_cast<std::uint16_t>(c.age + 52);
    return core::klass_name(target);
}

}  // namespace wiz::game
