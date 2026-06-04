#pragma once

#include <cstdint>

// Replaces Apple II UCSD Pascal 3-part long arithmetic (ADDLONGS / MULTLONG / etc.).
// The original packed three 16-bit words to simulate up to ~2^48 unsigned;
// here we just use int64_t which is bigger than anything Wizardry I ever needed.

namespace wiz::core {

using long_t = std::int64_t;

inline long_t add_longs(long_t a, long_t b) noexcept { return a + b; }
inline long_t sub_longs(long_t a, long_t b) noexcept { return a - b; }
inline long_t mul_long(long_t a, long_t b) noexcept { return a * b; }
inline long_t div_long(long_t a, long_t b) noexcept { return b == 0 ? 0 : a / b; }

}  // namespace wiz::core
