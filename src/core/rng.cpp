#include "core/rng.h"

namespace wiz::core {

namespace {
constexpr std::uint32_t rotl(std::uint32_t x, int k) noexcept {
    return (x << k) | (x >> (32 - k));
}

// splitmix32 — seed expander
std::uint32_t splitmix(std::uint64_t& s) noexcept {
    s += 0x9E37'79B9'7F4A'7C15ull;
    std::uint64_t z = s;
    z = (z ^ (z >> 30)) * 0xBF58'476D'1CE4'E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D0'49BB'1331'11EBull;
    return static_cast<std::uint32_t>(z ^ (z >> 31));
}
}  // namespace

void Rng::reseed(std::uint64_t seed) noexcept {
    s_[0] = splitmix(seed);
    s_[1] = splitmix(seed);
    s_[2] = splitmix(seed);
    s_[3] = splitmix(seed);
}

std::uint32_t Rng::next_u32() noexcept {
    const std::uint32_t result = rotl(s_[0] + s_[3], 7) + s_[0];
    const std::uint32_t t = s_[1] << 9;
    s_[2] ^= s_[0];
    s_[3] ^= s_[1];
    s_[1] ^= s_[2];
    s_[0] ^= s_[3];
    s_[2] ^= t;
    s_[3] = rotl(s_[3], 11);
    return result;
}

int Rng::range(int lo, int hi) noexcept {
    if (hi <= lo) return lo;
    std::uint32_t span = static_cast<std::uint32_t>(hi - lo + 1);
    return lo + static_cast<int>(next_u32() % span);
}

int Rng::dice(int n, int sides) noexcept {
    int total = 0;
    for (int i = 0; i < n; ++i) total += range(1, sides);
    return total;
}

Rng& global_rng() noexcept {
    static Rng rng;
    return rng;
}

}  // namespace wiz::core
