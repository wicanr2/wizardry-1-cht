#include "render/ui.h"

namespace wiz::render {

UI::UI(SDL_Renderer* r, const Font* title, const Font* body, const Font* small)
    : r_(r), title_(title), body_(body), small_(small) {}

void UI::draw_line(std::string_view utf8, int x, int y, SDL_Color color, Align align) const {
    draw_text(r_, *body_, utf8, x, y, color, align);
}

void UI::clear() const {
    SDL_SetRenderDrawColor(r_, theme_.bg.r, theme_.bg.g, theme_.bg.b, 255);
    SDL_RenderClear(r_);
}

void UI::present() const { SDL_RenderPresent(r_); }

void UI::draw_frame(int x, int y, int w, int h, bool filled) const {
    if (filled) {
        SDL_SetRenderDrawColor(r_, theme_.panel.r, theme_.panel.g, theme_.panel.b, 255);
        SDL_Rect bg{x, y, w, h};
        SDL_RenderFillRect(r_, &bg);
    }
    SDL_SetRenderDrawColor(r_, theme_.border.r, theme_.border.g, theme_.border.b, 255);
    SDL_Rect outline{x, y, w, h};
    SDL_RenderDrawRect(r_, &outline);
    SDL_Rect outline2{x + 1, y + 1, w - 2, h - 2};
    SDL_RenderDrawRect(r_, &outline2);
}

void UI::draw_title_bar(std::string_view text) const {
    int win_w = 0, win_h = 0;
    SDL_GetRendererOutputSize(r_, &win_w, &win_h);
    SDL_SetRenderDrawColor(r_, theme_.border.r, theme_.border.g, theme_.border.b, 255);
    SDL_Rect top{40, 30, win_w - 80, 2};
    SDL_RenderFillRect(r_, &top);
    draw_text(r_, *title_, text, win_w / 2, 45, theme_.accent, Align::Center);
}

void UI::draw_status_bar(std::string_view text) const {
    int win_w = 0, win_h = 0;
    SDL_GetRendererOutputSize(r_, &win_w, &win_h);
    SDL_SetRenderDrawColor(r_, theme_.border.r, theme_.border.g, theme_.border.b, 255);
    SDL_Rect bottom{40, win_h - 50, win_w - 80, 2};
    SDL_RenderFillRect(r_, &bottom);
    draw_text(r_, *small_, text, win_w / 2, win_h - 30, theme_.dim, Align::Center);
}

int UI::draw_menu(int x, int y, int w, const std::vector<MenuItem>& items,
                  int hovered_index) const {
    const int line = body_->line_height() + 8;
    const int pad = 10;
    for (std::size_t i = 0; i < items.size(); ++i) {
        const auto& it = items[i];
        SDL_Color c = it.enabled ? theme_.text : theme_.dim;
        if (static_cast<int>(i) == hovered_index) {
            SDL_SetRenderDrawColor(r_, theme_.highlight.r, theme_.highlight.g,
                                   theme_.highlight.b, 220);
            SDL_Rect hl{x, y + static_cast<int>(i) * line - 2, w, line - 2};
            SDL_RenderFillRect(r_, &hl);
            c = SDL_Color{255, 255, 255, 255};
        }
        std::string label;
        if (it.hotkey) {
            label.push_back('[');
            label.push_back(it.hotkey);
            label.append("] ");
        }
        label.append(it.label);
        draw_text(r_, *body_, label, x + pad, y + static_cast<int>(i) * line, c);
    }
    return -1;
}

void UI::draw_message_panel(int x, int y, int w, int h,
                            const std::vector<std::string>& lines) const {
    draw_frame(x, y, w, h);
    const int pad = 12;
    const int line = body_->line_height() + 4;
    const int max_rows = (h - 2 * pad) / line;
    if (max_rows <= 0) return;

    // Show the most-recent rows; oldest at top, newest at bottom.
    int first = static_cast<int>(lines.size()) - max_rows;
    if (first < 0) first = 0;
    int yy = y + pad;
    for (std::size_t i = first; i < lines.size(); ++i) {
        draw_text(r_, *body_, lines[i], x + pad, yy, theme_.text);
        yy += line;
    }
}

}  // namespace wiz::render
