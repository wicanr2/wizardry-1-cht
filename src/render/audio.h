#pragma once

#include <string>

namespace wiz::render {

// Audio subsystem — wraps SDL_mixer for BGM playback.
// If init fails (e.g. headless CI), all calls are no-ops.

bool audio_init();
void audio_shutdown();

// Start playing a music file (looped). If the same track is already playing,
// this is a no-op. Pass an empty string to stop music.
void play_music(const std::string& path, int loops = -1);

// Fade out current music over ms milliseconds.
void fade_out_music(int ms = 500);

// Set music volume 0..128.
void set_music_volume(int v);

[[nodiscard]] bool audio_available();

// Short sound effects (Mix_Chunk). Loaded lazily, cached by path.
// Pass empty string to clear the cache.
void play_sfx(const std::string& path);
void clear_sfx_cache();

// Helper enums to keep call sites short.
enum class Sfx {
    SwordHit,    // melee connects
    SwordMiss,   // melee misses
    SpellCast,   // any spell start
    SpellFire,   // HALITO/MAHALITO etc.
    SpellHeal,   // DIOS/DIAL/MADI
    MonsterDie,
    PartyDamage,
    Footstep,
    DoorOpen,
    MenuMove,    // ↑↓ tick
    MenuPick,    // Enter
};
void play(Sfx s);  // resolves enum → path automatically

}  // namespace wiz::render
