#pragma once

#include <SDL.h>

#include <string>
#include <string_view>
#include <vector>

#include "render/text.h"

namespace wiz::render {

struct MenuItem {
    std::string label;
    char hotkey = 0;
    bool enabled = true;
};

struct Theme {
    SDL_Color bg{12, 14, 22, 255};
    SDL_Color panel{22, 26, 38, 255};
    SDL_Color border{90, 70, 30, 255};
    SDL_Color text{235, 235, 235, 255};
    SDL_Color dim{120, 120, 130, 255};
    SDL_Color accent{230, 180, 60, 255};
    SDL_Color highlight{60, 110, 200, 255};
};

class UI {
   public:
    UI(SDL_Renderer* r, const Font* title, const Font* body, const Font* small);

    void clear() const;
    void present() const;

    void draw_frame(int x, int y, int w, int h, bool filled = true) const;
    void draw_title_bar(std::string_view text) const;
    void draw_status_bar(std::string_view text) const;

    // Returns hotkey/index chosen via SDL event; -1 if none yet.
    int draw_menu(int x, int y, int w, const std::vector<MenuItem>& items,
                  int hovered_index) const;

    void draw_message_panel(int x, int y, int w, int h,
                            const std::vector<std::string>& lines) const;

    [[nodiscard]] const Theme& theme() const noexcept { return theme_; }
    [[nodiscard]] const Font& body_font() const noexcept { return *body_; }
    [[nodiscard]] const Font& title_font() const noexcept { return *title_; }
    [[nodiscard]] const Font& small_font() const noexcept { return *small_; }
    [[nodiscard]] SDL_Renderer* renderer() const noexcept { return r_; }

    void draw_line(std::string_view utf8, int x, int y, SDL_Color color,
                   Align align = Align::Left) const;

   private:
    SDL_Renderer* r_;
    const Font* title_;
    const Font* body_;
    const Font* small_;
    Theme theme_;
};

}  // namespace wiz::render
