#pragma once

#include <cstdint>

namespace wiz::core {

// Deterministic RNG replacing the Apple II 6502 RANDOM routine.
// The Apple II original used a memory-poking LFSR; we use xoshiro128++ which
// has a tiny state and good distribution for d20-style dice rolls.
class Rng {
   public:
    explicit Rng(std::uint64_t seed = 0xDEAD'BEEF'CAFE'BABEull) noexcept { reseed(seed); }

    void reseed(std::uint64_t seed) noexcept;
    std::uint32_t next_u32() noexcept;

    int range(int lo, int hi) noexcept;   // inclusive [lo, hi]
    int dice(int n, int sides) noexcept;  // nDsides

   private:
    std::uint32_t s_[4]{};
};

Rng& global_rng() noexcept;

}  // namespace wiz::core
