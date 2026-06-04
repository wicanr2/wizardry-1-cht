#pragma once

#include <array>
#include <string>

#include "core/character.h"

namespace wiz::save {

// Saved character roster — JSON v1 format. Decoupled from on-disk Apple II
// layout; we store everything we need to reconstruct a Character.

struct Roster {
    static constexpr std::size_t kMaxChars = 200;  // matches Apple II RECPERDK[ZCHAR]
    std::array<core::Character, kMaxChars> chars{};
    std::size_t used = 0;
};

bool save(const Roster& roster, const std::string& path);
bool load(Roster& roster, const std::string& path);

}  // namespace wiz::save
