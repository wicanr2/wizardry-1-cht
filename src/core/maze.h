#pragma once

#include <array>
#include <cstdint>

namespace wiz::core {

enum class Wall : std::uint8_t { Open = 0, Wall = 1, Door = 2, HiddenDoor = 3 };

enum class SquareFeature : std::uint8_t {
    None = 0,
    Stairs,
    Chute,
    Spinner,
    Pit,
    Teleporter,
    Encounter,
    Message,
    Elevator,
    Rock,
    Fizzle,
    SquareCount,
};

// Mirrors TMAZE from WIZ.TEXT lines 139-162.
struct MazeLevel {
    static constexpr int kSize = 20;
    // Per-cell wall on each side
    std::array<std::array<Wall, kSize>, kSize> west{};
    std::array<std::array<Wall, kSize>, kSize> south{};
    std::array<std::array<Wall, kSize>, kSize> east{};
    std::array<std::array<Wall, kSize>, kSize> north{};
    std::array<std::array<std::uint8_t, kSize>, kSize> fights{};      // 0/1
    std::array<std::array<std::uint8_t, kSize>, kSize> sqr_extra{};   // 4-bit index into sqre_type
    // Dark zone (no natural light). Without an active MILWA / LOMILWA, the
    // maze_view skips far-depth walls and the auto-map hides the cell.
    std::array<std::array<bool, kSize>, kSize> dark_zone{};
    std::array<SquareFeature, 16> sqre_type{};
    std::array<std::int16_t, 16> aux0{};
    std::array<std::int16_t, 16> aux1{};
    std::array<std::int16_t, 16> aux2{};
    std::array<std::uint8_t, 3> enmy_calc{};  // monster spawn probability
    std::uint8_t level_number = 1;

    // Auto-map: bitfield, cell (x,y) visited if visited[y][x]
    std::array<std::array<bool, kSize>, kSize> visited{};
};

}  // namespace wiz::core
