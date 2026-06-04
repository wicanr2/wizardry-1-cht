#include "render/text.h"

#include <SDL_ttf.h>

#include <cstdio>
#include <string>

namespace wiz::render {

Font::~Font() {
    if (font_) TTF_CloseFont(font_);
}

bool Font::load(const std::string& path, int point_size) {
    if (font_) {
        TTF_CloseFont(font_);
        font_ = nullptr;
    }
    font_ = TTF_OpenFont(path.c_str(), point_size);
    if (!font_) {
        std::fprintf(stderr, "[font] TTF_OpenFont failed for %s: %s\n",
                     path.c_str(), TTF_GetError());
        return false;
    }
    TTF_SetFontHinting(font_, TTF_HINTING_LIGHT);
    return true;
}

int Font::line_height() const noexcept {
    return font_ ? TTF_FontLineSkip(font_) : 0;
}

void draw_text(SDL_Renderer* renderer, const Font& font, std::string_view utf8,
               int x, int y, SDL_Color color, Align align) {
    if (!font.handle() || utf8.empty()) return;
    std::string buf{utf8};
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font.handle(), buf.c_str(), color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_Rect dst{x, y, surf->w, surf->h};
    switch (align) {
        case Align::Center: dst.x = x - surf->w / 2; break;
        case Align::Right:  dst.x = x - surf->w; break;
        case Align::Left: default: break;
    }
    SDL_FreeSurface(surf);
    if (tex) {
        SDL_RenderCopy(renderer, tex, nullptr, &dst);
        SDL_DestroyTexture(tex);
    }
}

int measure_text_width(const Font& font, std::string_view utf8) {
    if (!font.handle() || utf8.empty()) return 0;
    int w = 0, h = 0;
    std::string buf{utf8};
    TTF_SizeUTF8(font.handle(), buf.c_str(), &w, &h);
    return w;
}

}  // namespace wiz::render
