#pragma once

#include <SDL.h>

#include <string>

namespace wiz::render {

struct WindowOptions {
    int width = 1280;
    int height = 720;
    std::string title = "Wizardry";
};

class Window {
   public:
    Window() = default;
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool create(const WindowOptions& opts);
    void destroy();

    [[nodiscard]] SDL_Window* sdl_window() const noexcept { return window_; }
    [[nodiscard]] SDL_Renderer* renderer() const noexcept { return renderer_; }
    [[nodiscard]] int width() const noexcept { return opts_.width; }
    [[nodiscard]] int height() const noexcept { return opts_.height; }

   private:
    WindowOptions opts_{};
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
};

}  // namespace wiz::render
