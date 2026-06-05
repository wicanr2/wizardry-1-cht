#include "render/audio.h"

#include <SDL.h>
#include <SDL_mixer.h>

#include <cstdio>
#include <string>

namespace wiz::render {

namespace {
Mix_Music* g_current = nullptr;
std::string g_current_path;
bool g_ready = false;
int g_volume = 64;  // 0..128, half by default
}  // namespace

bool audio_init() {
    if (g_ready) return true;
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        std::fprintf(stderr, "[audio] SDL_INIT_AUDIO failed: %s\n", SDL_GetError());
        return false;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) != 0) {
        std::fprintf(stderr, "[audio] Mix_OpenAudio failed: %s\n", Mix_GetError());
        return false;
    }
    int flags = MIX_INIT_OGG | MIX_INIT_MP3;
    int got = Mix_Init(flags);
    if ((got & flags) != flags) {
        std::fprintf(stderr, "[audio] Mix_Init partial: requested=%x got=%x\n",
                     flags, got);
        // Continue anyway — some formats might still work
    }
    Mix_VolumeMusic(g_volume);
    g_ready = true;
    std::fprintf(stderr, "[audio] initialized (volume %d/128)\n", g_volume);
    return true;
}

void audio_shutdown() {
    if (!g_ready) return;
    if (g_current) {
        Mix_HaltMusic();
        Mix_FreeMusic(g_current);
        g_current = nullptr;
    }
    Mix_CloseAudio();
    Mix_Quit();
    g_ready = false;
}

void play_music(const std::string& path, int loops) {
    if (!g_ready) return;
    if (path.empty()) {
        Mix_HaltMusic();
        if (g_current) { Mix_FreeMusic(g_current); g_current = nullptr; }
        g_current_path.clear();
        return;
    }
    if (path == g_current_path && Mix_PlayingMusic()) return;

    if (g_current) {
        Mix_HaltMusic();
        Mix_FreeMusic(g_current);
        g_current = nullptr;
    }
    g_current = Mix_LoadMUS(path.c_str());
    if (!g_current) {
        std::fprintf(stderr, "[audio] cannot load %s: %s\n",
                     path.c_str(), Mix_GetError());
        return;
    }
    if (Mix_PlayMusic(g_current, loops) != 0) {
        std::fprintf(stderr, "[audio] play failed for %s: %s\n",
                     path.c_str(), Mix_GetError());
        Mix_FreeMusic(g_current);
        g_current = nullptr;
        return;
    }
    g_current_path = path;
}

void fade_out_music(int ms) {
    if (!g_ready) return;
    Mix_FadeOutMusic(ms);
}

void set_music_volume(int v) {
    if (v < 0) v = 0;
    if (v > 128) v = 128;
    g_volume = v;
    if (g_ready) Mix_VolumeMusic(v);
}

bool audio_available() { return g_ready; }

}  // namespace wiz::render
