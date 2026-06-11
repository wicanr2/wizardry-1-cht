#include "render/palette.h"

namespace wiz::render {

MazePalette maze_palette_for(theme::Theme t) {
    switch (t) {
        case theme::Theme::PCECD:
            // Warm 16-bit stone — gold walls, red doors, blue floor.
            return MazePalette{
                {220, 200, 150, 255},  // wall: aged sandstone
                {200,  90,  60, 255},  // door: crimson lacquer
                {130,  95,  45, 255},  // frame: bronze
                { 12,  16,  26, 255},  // floor: deep dungeon blue
                { 28,  24,  34, 255},  // ceiling: dim slate
                {180, 150,  90, 255},  // far_wall: aged gold
            };
        case theme::Theme::Mono:
            // 1-bit Apple II vector — pure white wireframe on black.
            return MazePalette{
                {255, 255, 255, 255},
                {200, 200, 200, 255},
                {255, 255, 255, 255},
                {  0,   0,   0, 255},
                {  0,   0,   0, 255},
                {255, 255, 255, 255},
            };
        case theme::Theme::Outline:
            // Edge-only line art — slightly grey walls, charcoal void.
            return MazePalette{
                {210, 210, 210, 255},
                {245, 245, 245, 255},
                {235, 235, 235, 255},
                { 12,  12,  18, 255},
                { 12,  12,  18, 255},
                {210, 210, 210, 255},
            };
        case theme::Theme::Sepia:
            // CGA / DOS era — wheat walls, amber doors, dark-brown void.
            return MazePalette{
                {220, 180, 120, 255},
                {255, 140,  70, 255},
                {150,  95,  40, 255},
                { 22,  14,   8, 255},
                { 32,  22,  12, 255},
                {180, 140,  80, 255},
            };
        case theme::Theme::PC98:
            // PC-98 digital palette — cyan walls, magenta doors, navy void.
            return MazePalette{
                {120, 220, 255, 255},
                {255,  80, 180, 255},
                {200, 200, 200, 255},
                {  8,   8,  32, 255},
                { 16,  16,  48, 255},
                { 90, 180, 230, 255},
            };
        case theme::Theme::WSC:
            // WonderSwan Color — green LCD-tint walls, yellow doors.
            return MazePalette{
                {180, 230, 180, 255},
                {240, 220,  80, 255},
                { 80, 120,  80, 255},
                { 14,  20,  14, 255},
                { 22,  30,  22, 255},
                {150, 200, 150, 255},
            };
        case theme::Theme::Mac:
            // Classic Mac monochrome with accent blue — blueprint feel.
            return MazePalette{
                {100, 140, 200, 255},
                { 60,  90, 200, 255},
                {120, 120, 120, 255},
                {200, 200, 200, 255},
                {180, 180, 180, 255},
                { 80, 110, 170, 255},
            };
        case theme::Theme::Count:
            break;
    }
    // Defensive fallback = PCECD.
    return maze_palette_for(theme::Theme::PCECD);
}

}  // namespace wiz::render
