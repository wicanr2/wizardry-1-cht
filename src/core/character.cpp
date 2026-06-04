#include "core/character.h"

namespace wiz::core {

const char* race_name(Race r) {
    switch (r) {
        case Race::Human:  return "human";
        case Race::Elf:    return "elf";
        case Race::Dwarf:  return "dwarf";
        case Race::Gnome:  return "gnome";
        case Race::Hobbit: return "hobbit";
        default: return "unknown";
    }
}

const char* klass_name(Klass k) {
    switch (k) {
        case Klass::Fighter: return "fighter";
        case Klass::Mage:    return "mage";
        case Klass::Priest:  return "priest";
        case Klass::Thief:   return "thief";
        case Klass::Bishop:  return "bishop";
        case Klass::Samurai: return "samurai";
        case Klass::Lord:    return "lord";
        case Klass::Ninja:   return "ninja";
        default: return "unknown";
    }
}

const char* alignment_name(Alignment a) {
    switch (a) {
        case Alignment::Good:    return "good";
        case Alignment::Neutral: return "neutral";
        case Alignment::Evil:    return "evil";
        default: return "unknown";
    }
}

const char* status_name(Status s) {
    switch (s) {
        case Status::Ok:        return "ok";
        case Status::Afraid:    return "afraid";
        case Status::Asleep:    return "asleep";
        case Status::Paralyzed: return "paralyzed";
        case Status::Stoned:    return "stoned";
        case Status::Dead:      return "dead";
        case Status::Ashes:     return "ashes";
        case Status::Lost:      return "lost";
        default: return "unknown";
    }
}

}  // namespace wiz::core
