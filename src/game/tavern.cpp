#include "game/tavern.h"

#include <SDL.h>

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

namespace wiz::game {

namespace {

constexpr int kPadX = 80;
constexpr int kPadY = 110;

enum class Focus { RosterList, PartyList };

struct TavernUI {
    Focus focus = Focus::RosterList;
    int roster_cursor = 0;
    int party_cursor = 0;
    int roster_scroll = 0;
};

TavernUI& ui_state() { static TavernUI s; return s; }

bool in_party(const State& state, int roster_idx) {
    for (int i = 0; i < state.party.count; ++i) {
        if (state.party.roster_index[i] == roster_idx) return true;
    }
    return false;
}

bool add_to_party(State& state, int roster_idx) {
    if (state.party.count >= 6) return false;
    if (in_party(state, roster_idx)) return false;
    state.party.roster_index[state.party.count++] = roster_idx;
    return true;
}

bool remove_from_party(State& state, int party_idx) {
    if (party_idx < 0 || party_idx >= state.party.count) return false;
    for (int i = party_idx; i < state.party.count - 1; ++i) {
        state.party.roster_index[i] = state.party.roster_index[i + 1];
    }
    state.party.roster_index[state.party.count - 1] = -1;
    --state.party.count;
    return true;
}

void draw_tavern(const State& state, const render::UI& ui, const TavernUI& s) {
    ui.clear();
    ui.draw_title_bar("吉爾伽美什酒館");

    // Left panel: full roster
    const int left_x = kPadX;
    const int left_w = 600;
    const int panel_h = 520;
    ui.draw_frame(left_x, kPadY, left_w, panel_h);
    SDL_Color title_color = s.focus == Focus::RosterList
                                ? ui.theme().accent
                                : ui.theme().dim;
    render::draw_text(ui.renderer(), ui.body_font(),
                      "角色名冊（按 A 加入隊伍）",
                      left_x + 14, kPadY + 14, title_color);

    const int line_h = ui.body_font().line_height() + 8;
    const int rows = (panel_h - 60) / line_h;
    int first = s.roster_scroll;
    int last = std::min<int>(first + rows, static_cast<int>(state.roster.used));

    int yy = kPadY + 50;
    for (int i = first; i < last; ++i) {
        const auto& c = state.roster.chars[i];
        bool joined = in_party(state, i);
        SDL_Color col = ui.theme().text;
        if (s.focus == Focus::RosterList && i == s.roster_cursor) {
            SDL_SetRenderDrawColor(ui.renderer(),
                                   ui.theme().highlight.r,
                                   ui.theme().highlight.g,
                                   ui.theme().highlight.b, 200);
            SDL_Rect hl{left_x + 8, yy - 4, left_w - 16, line_h};
            SDL_RenderFillRect(ui.renderer(), &hl);
            col = SDL_Color{255, 255, 255, 255};
        }
        if (joined) col = ui.theme().dim;

        char buf[200];
        std::snprintf(buf, sizeof(buf), "%2d. %-14s Lv%d %-6s HP %d/%d %s",
                      i + 1, c.name.c_str(),
                      int(c.char_level),
                      core::klass_name(c.klass),
                      int(c.hp_left), int(c.hp_max),
                      joined ? "(已在隊)" : "");
        render::draw_text(ui.renderer(), ui.body_font(), buf,
                          left_x + 14, yy, col);
        yy += line_h;
    }
    if (state.roster.used == 0) {
        render::draw_text(ui.renderer(), ui.body_font(),
                          "（名冊空無一人 — 去訓練場創角色）",
                          left_x + 14, kPadY + 60, ui.theme().dim);
    }

    // Right panel: current party
    const int right_x = left_x + left_w + 24;
    const int right_w = 1280 - right_x - kPadX;
    ui.draw_frame(right_x, kPadY, right_w, panel_h);
    title_color = s.focus == Focus::PartyList
                      ? ui.theme().accent
                      : ui.theme().dim;
    render::draw_text(ui.renderer(), ui.body_font(),
                      "目前隊伍（按 R 移除）",
                      right_x + 14, kPadY + 14, title_color);

    yy = kPadY + 50;
    for (int i = 0; i < state.party.count; ++i) {
        int ri = state.party.roster_index[i];
        if (ri < 0) continue;
        const auto& c = state.roster.chars[ri];
        SDL_Color col = ui.theme().text;
        if (s.focus == Focus::PartyList && i == s.party_cursor) {
            SDL_SetRenderDrawColor(ui.renderer(),
                                   ui.theme().highlight.r,
                                   ui.theme().highlight.g,
                                   ui.theme().highlight.b, 200);
            SDL_Rect hl{right_x + 8, yy - 4, right_w - 16, line_h};
            SDL_RenderFillRect(ui.renderer(), &hl);
            col = SDL_Color{255, 255, 255, 255};
        }
        char buf[160];
        std::snprintf(buf, sizeof(buf), "%d. %-12s %-6s HP %d/%d",
                      i + 1, c.name.c_str(),
                      core::klass_name(c.klass),
                      int(c.hp_left), int(c.hp_max));
        render::draw_text(ui.renderer(), ui.body_font(), buf,
                          right_x + 14, yy, col);
        yy += line_h;
    }
    if (state.party.count == 0) {
        render::draw_text(ui.renderer(), ui.body_font(),
                          "（無隊員）",
                          right_x + 14, kPadY + 60, ui.theme().dim);
    }

    if (!state.message_log.empty()) {
        render::draw_text(ui.renderer(), ui.body_font(),
                          std::string("» ") + state.message_log.back(),
                          kPadX + 8, kPadY + panel_h + 20, ui.theme().accent);
    }

    ui.draw_status_bar("Tab 切換面板  ↑↓ 選擇  A 加入  R 移除  ESC 離開");
    ui.present();
}

}  // namespace

bool tavern_tick(State& state, const SDL_Event* event, const render::UI& ui) {
    auto& s = ui_state();
    int roster_used = static_cast<int>(state.roster.used);
    if (s.roster_cursor >= roster_used && roster_used > 0) s.roster_cursor = roster_used - 1;
    if (s.party_cursor >= state.party.count && state.party.count > 0)
        s.party_cursor = state.party.count - 1;

    if (event && event->type == SDL_KEYDOWN) {
        SDL_Keycode k = event->key.keysym.sym;
        if (k == SDLK_ESCAPE) {
            state.change_scene(Scene::Castle);
            return true;
        }
        if (k == SDLK_TAB) {
            s.focus = (s.focus == Focus::RosterList) ? Focus::PartyList : Focus::RosterList;
        }
        if (k == SDLK_UP) {
            if (s.focus == Focus::RosterList && roster_used > 0) {
                s.roster_cursor = (s.roster_cursor - 1 + roster_used) % roster_used;
                if (s.roster_cursor < s.roster_scroll) s.roster_scroll = s.roster_cursor;
            } else if (s.focus == Focus::PartyList && state.party.count > 0) {
                s.party_cursor = (s.party_cursor - 1 + state.party.count) % state.party.count;
            }
        }
        if (k == SDLK_DOWN) {
            if (s.focus == Focus::RosterList && roster_used > 0) {
                s.roster_cursor = (s.roster_cursor + 1) % roster_used;
                const int rows = (520 - 60) / (ui.body_font().line_height() + 8);
                if (s.roster_cursor >= s.roster_scroll + rows)
                    s.roster_scroll = s.roster_cursor - rows + 1;
                if (s.roster_cursor == 0) s.roster_scroll = 0;
            } else if (s.focus == Focus::PartyList && state.party.count > 0) {
                s.party_cursor = (s.party_cursor + 1) % state.party.count;
            }
        }
        if (k == SDLK_a && roster_used > 0) {
            int ri = s.roster_cursor;
            if (in_party(state, ri)) {
                state.push_message("** 已在隊伍中 **");
            } else if (!add_to_party(state, ri)) {
                state.push_message("** 隊伍已滿（最多 6 人）**");
            } else {
                state.push_message(state.roster.chars[ri].name + " 加入隊伍。");
            }
        }
        // Shift + Up / Down — reorder party slots. Row 0..2 = front
        // (melee-eligible), 3..5 = back. Moving a Mage to front-row makes
        // them squishier; moving a Fighter back loses their melee swing.
        bool shift_held = (event->key.keysym.mod & KMOD_SHIFT) != 0;
        if (shift_held && s.focus == Focus::PartyList && state.party.count > 1) {
            int pi = s.party_cursor;
            int swap_with = -1;
            if (k == SDLK_UP   && pi > 0)                      swap_with = pi - 1;
            if (k == SDLK_DOWN && pi < state.party.count - 1)  swap_with = pi + 1;
            if (swap_with >= 0) {
                std::swap(state.party.roster_index[pi],
                          state.party.roster_index[swap_with]);
                s.party_cursor = swap_with;
                state.push_message("✦ 隊形已調整。");
            }
        }
        if (k == SDLK_r && state.party.count > 0) {
            int pi = s.party_cursor;
            int ri = state.party.roster_index[pi];
            std::string name = (ri >= 0) ? state.roster.chars[ri].name : "?";
            if (remove_from_party(state, pi)) {
                state.push_message(name + " 已離開隊伍。");
                if (s.party_cursor >= state.party.count && state.party.count > 0)
                    s.party_cursor = state.party.count - 1;
            }
        }
    }

    draw_tavern(state, ui, s);
    return true;
}

}  // namespace wiz::game
