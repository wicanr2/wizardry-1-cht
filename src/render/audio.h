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

}  // namespace wiz::render
