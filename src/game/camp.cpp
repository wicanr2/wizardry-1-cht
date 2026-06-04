#include "game/camp.h"

#include <SDL.h>

#include <cstdio>
#include <cstdlib>
#include <string>

#include "save/gamesave.h"

namespace wiz::game {

namespace {

constexpr int kPadX = 80;
constexpr int kPadY = 110;

enum CampOption { Save, Inspect, Reorder, Back, ToTitle, Count };

struct CampUI {
    int cursor = 0;
    int inspect_idx = 0;
    bool inspecting = false;
};

CampUI& ui_state() { static CampUI s; return s; }

const char* option_label(int o) {
    switch (o) {
        case Save:    return "[S] 存檔";
        case Inspect: return "[I] 檢視角色";
        case Reorder: return "[R] 重排隊伍";
        case Back:    return "[ESC] 回到迷宮";
        case ToTitle: return "[Q] 回到標題";
        default: return "";
    }
}

void draw_camp(State& state, const render::UI& ui, const CampUI& s) {
    ui.clear();
    ui.draw_title_bar("營地 — 迷宮中休息");

    // Left: action menu
    const int left_w = 380;
    ui.draw_frame(kPadX, kPadY, left_w, 360);
    render::draw_text(ui.renderer(), ui.body_font(),
                      "選擇行動", kPadX + 14, kPadY + 14, ui.theme().accent);
    const int line_h = ui.body_font().line_height() + 12;
    for (int i = 0; i < Count; ++i) {
        int yy = kPadY + 60 + i * line_h;
        SDL_Color col = ui.theme().text;
        if (i == s.cursor) {
            SDL_SetRenderDrawColor(ui.renderer(),
                                   ui.theme().highlight.r,
                                   ui.theme().highlight.g,
                                   ui.theme().highlight.b, 200);
            SDL_Rect hl{kPadX + 8, yy - 4, left_w - 16, line_h};
            SDL_RenderFillRect(ui.renderer(), &hl);
            col = SDL_Color{255, 255, 255, 255};
        }
        render::draw_text(ui.renderer(), ui.body_font(), option_label(i),
                          kPadX + 24, yy, col);
    }

    // Right: party detail or inspector
    const int right_x = kPadX + left_w + 24;
    const int right_w = 1280 - right_x - kPadX;
    const int right_h = 360;
    ui.draw_frame(right_x, kPadY, right_w, right_h);

    if (s.inspecting && s.inspect_idx >= 0 && s.inspect_idx < state.party.count) {
        int ri = state.party.roster_index[s.inspect_idx];
        if (ri >= 0) {
            const auto& c = state.roster.chars[ri];
            char buf[200];
            int yy = kPadY + 16;
            render::draw_text(ui.renderer(), ui.body_font(), c.name,
                              right_x + 14, yy, ui.theme().accent);
            yy += ui.body_font().line_height() + 6;
            std::snprintf(buf, sizeof(buf), "%s / %s / %s   Lv%d",
                          core::race_name(c.race),
                          core::klass_name(c.klass),
                          core::alignment_name(c.alignment),
                          int(c.char_level));
            render::draw_text(ui.renderer(), ui.body_font(), buf,
                              right_x + 14, yy, ui.theme().text);
            yy += ui.body_font().line_height() + 6;
            std::snprintf(buf, sizeof(buf), "HP %d/%d   AC %d   金 %lld   經驗 %lld",
                          int(c.hp_left), int(c.hp_max),
                          int(c.armor_class),
                          (long long)c.gold, (long long)c.experience);
            render::draw_text(ui.renderer(), ui.body_font(), buf,
                              right_x + 14, yy, ui.theme().text);
            yy += ui.body_font().line_height() + 10;
            std::snprintf(buf, sizeof(buf),
                          "力 %d  智 %d  信 %d  體 %d  敏 %d  幸 %d",
                          int(c.attr.strength), int(c.attr.iq),
                          int(c.attr.piety), int(c.attr.vitality),
                          int(c.attr.agility), int(c.attr.luck));
            render::draw_text(ui.renderer(), ui.body_font(), buf,
                              right_x + 14, yy, ui.theme().text);
            yy += ui.body_font().line_height() + 6;
            std::snprintf(buf, sizeof(buf), "年齡 %d 週   狀態：%s",
                          int(c.age), core::status_name(c.status));
            render::draw_text(ui.renderer(), ui.body_font(), buf,
                              right_x + 14, yy, ui.theme().text);
            yy += ui.body_font().line_height() + 14;
            std::snprintf(buf, sizeof(buf), "← → 切換  (%d / %d)",
                          s.inspect_idx + 1, state.party.count);
            render::draw_text(ui.renderer(), ui.small_font(), buf,
                              right_x + 14, yy, ui.theme().dim);
        }
    } else {
        render::draw_text(ui.renderer(), ui.body_font(), "目前隊伍",
                          right_x + 14, kPadY + 14, ui.theme().accent);
        int yy = kPadY + 14 + ui.body_font().line_height() + 6;
        for (int i = 0; i < state.party.count; ++i) {
            int ri = state.party.roster_index[i];
            if (ri < 0) continue;
            const auto& c = state.roster.chars[ri];
            char line[160];
            std::snprintf(line, sizeof(line),
                          "%d. %-12s %-6s HP %d/%d",
                          i + 1, c.name.c_str(),
                          core::klass_name(c.klass),
                          int(c.hp_left), int(c.hp_max));
            render::draw_text(ui.renderer(), ui.body_font(), line,
                              right_x + 14, yy, ui.theme().text);
            yy += ui.body_font().line_height() + 4;
        }

        // Camera info
        yy = kPadY + right_h - 70;
        char buf[100];
        std::snprintf(buf, sizeof(buf), "位置：B%dF X:%d Y:%d  面向 %s",
                      state.camera.level, state.camera.x, state.camera.y,
                      state.camera.facing == render::Facing::North ? "北" :
                      state.camera.facing == render::Facing::East ? "東" :
                      state.camera.facing == render::Facing::South ? "南" : "西");
        render::draw_text(ui.renderer(), ui.small_font(), buf,
                          right_x + 14, yy, ui.theme().dim);
    }

    if (!state.message_log.empty()) {
        render::draw_text(ui.renderer(), ui.body_font(),
                          std::string("» ") + state.message_log.back(),
                          kPadX + 8, kPadY + 360 + 24, ui.theme().accent);
    }

    ui.draw_status_bar(s.inspecting
                           ? "← → 切換角色   ESC 返回選單"
                           : "↑↓ 選項  Enter / 字母 執行  ESC 回迷宮");
    ui.present();
}

}  // namespace

std::string default_save_path() {
    const char* home = std::getenv("HOME");
    if (home && *home) return std::string(home) + "/.config/wizardry-cht/save.json";
    return "save.json";
}

bool camp_tick(State& state, const SDL_Event* event, const render::UI& ui) {
    auto& s = ui_state();

    if (event && event->type == SDL_KEYDOWN) {
        SDL_Keycode k = event->key.keysym.sym;

        if (s.inspecting) {
            if (k == SDLK_ESCAPE) { s.inspecting = false; }
            else if (k == SDLK_LEFT && state.party.count > 0) {
                s.inspect_idx = (s.inspect_idx - 1 + state.party.count) % state.party.count;
            } else if (k == SDLK_RIGHT && state.party.count > 0) {
                s.inspect_idx = (s.inspect_idx + 1) % state.party.count;
            }
        } else {
            if (k == SDLK_ESCAPE) {
                state.change_scene(Scene::Maze);
                return true;
            }
            if (k == SDLK_UP) s.cursor = (s.cursor - 1 + Count) % Count;
            if (k == SDLK_DOWN) s.cursor = (s.cursor + 1) % Count;

            auto run = [&](int opt) {
                switch (opt) {
                    case Save: {
                        std::string p = default_save_path();
                        // mkdir parent
                        auto slash = p.find_last_of('/');
                        if (slash != std::string::npos) {
                            std::string dir = p.substr(0, slash);
                            std::string cmd = "mkdir -p '" + dir + "'";
                            (void)std::system(cmd.c_str());
                        }
                        if (save::save_game(state, p)) {
                            state.push_message(std::string("✦ 已存檔：") + p);
                        } else {
                            state.push_message("** 存檔失敗 **");
                        }
                        break;
                    }
                    case Inspect:
                        s.inspecting = true;
                        s.inspect_idx = 0;
                        break;
                    case Reorder: {
                        // Simple rotation — first member goes to back
                        if (state.party.count > 1) {
                            int first = state.party.roster_index[0];
                            for (int i = 0; i < state.party.count - 1; ++i)
                                state.party.roster_index[i] = state.party.roster_index[i + 1];
                            state.party.roster_index[state.party.count - 1] = first;
                            state.push_message("隊伍順序：第一名移到隊尾。");
                        }
                        break;
                    }
                    case Back:
                        state.change_scene(Scene::Maze);
                        break;
                    case ToTitle:
                        state.change_scene(Scene::Title);
                        state.maze_loaded = false;
                        break;
                }
            };

            if (k == SDLK_RETURN || k == SDLK_SPACE) run(s.cursor);
            else if (k == SDLK_s) run(Save);
            else if (k == SDLK_i) run(Inspect);
            else if (k == SDLK_r) run(Reorder);
            else if (k == SDLK_q) run(ToTitle);
        }
    }

    draw_camp(state, ui, s);
    return true;
}

}  // namespace wiz::game
