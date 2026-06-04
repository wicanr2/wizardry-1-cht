#pragma once

#include <SDL.h>
#include <SDL_ttf.h>

#include <string>
#include <string_view>

namespace wiz::render {

class Font {
   public:
    Font() = default;
    ~Font();
    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;

    bool load(const std::string& path, int point_size);
    [[nodiscard]] TTF_Font* handle() const noexcept { return font_; }
    [[nodiscard]] int line_height() const noexcept;

   private:
    TTF_Font* font_ = nullptr;
};

enum class Align { Left, Center, Right };

void draw_text(SDL_Renderer* renderer, const Font& font,
               std::string_view utf8, int x, int y, SDL_Color color,
               Align align = Align::Left);

int measure_text_width(const Font& font, std::string_view utf8);

}  // namespace wiz::render
