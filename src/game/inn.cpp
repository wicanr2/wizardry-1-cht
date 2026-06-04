#include "game/inn.h"

#include <SDL.h>

#include <array>
#include <cstdio>
#include <string>

#include "core/rules.h"

namespace wiz::game {

namespace {

constexpr int kPadX = 80;
constexpr int kPadY = 110;

struct Room {
    const char* name;
    long long price;     // per week
    int hp_per_week;     // HP recovered per week
    int weeks_per_level; // how many weeks to age per stay (1)
};

constexpr std::array<Room, 5> kRooms = {{
    {"馬廄 (免費)",        0,    1,  1},
    {"簡易床位",         10,    3,  1},
    {"經濟客房",         50,    7,  1},
    {"商人套房",        200,   10,  1},
    {"皇家套房",        500,   15,  1},
}};

struct InnUI { int member = 0; int room = 0; };
InnUI& inn_state() { static InnUI s; return s; }

struct TempleUI {
    int member = 0;
    enum Action { Heal, CurePoison, CureParalysis, CureStone, Resurrect, Count } action = Heal;
};
TempleUI& temple_state() { static TempleUI s; return s; }

void draw_inn(State& state, const render::UI& ui, const InnUI& s) {
    ui.clear();
    ui.draw_title_bar("冒險者旅館");

    // Left: party member picker
    const int left_w = 380;
    ui.draw_frame(kPadX, kPadY, left_w, 520);
    render::draw_text(ui.renderer(), ui.body_font(), "投宿者",
                      kPadX + 14, kPadY + 14, ui.theme().accent);
    int yy = kPadY + 14 + ui.body_font().line_height() + 6;
    for (int i = 0; i < state.party.count; ++i) {
        int ri = state.party.roster_index[i];
        if (ri < 0) continue;
        const auto& c = state.roster.chars[ri];
        SDL_Color col = (i == s.member) ? SDL_Color{255,255,255,255} : ui.theme().text;
        if (i == s.member) {
            SDL_SetRenderDrawColor(ui.renderer(),
                                   ui.theme().highlight.r,
                                   ui.theme().highlight.g,
                                   ui.theme().highlight.b, 200);
            SDL_Rect hl{kPadX + 8, yy - 4, left_w - 16, ui.body_font().line_height() + 8};
            SDL_RenderFillRect(ui.renderer(), &hl);
        }
        char line[120];
        std::snprintf(line, sizeof(line), "%d. %-10s  HP %d/%d  金 %lld",
                      i + 1, c.name.c_str(),
                      int(c.hp_left), int(c.hp_max),
                      (long long)c.gold);
        render::draw_text(ui.renderer(), ui.body_font(), line,
                          kPadX + 24, yy, col);
        yy += ui.body_font().line_height() + 8;
    }
    render::draw_text(ui.renderer(), ui.small_font(),
                      "Tab 切換投宿者", kPadX + 14, kPadY + 520 - 30, ui.theme().dim);

    // Right: room list
    const int right_x = kPadX + left_w + 24;
    const int right_w = 1280 - right_x - kPadX;
    ui.draw_frame(right_x, kPadY, right_w, 520);
    render::draw_text(ui.renderer(), ui.body_font(),
                      "選擇房間（Enter 投宿一週）",
                      right_x + 14, kPadY + 14, ui.theme().accent);

    const int line_h = ui.body_font().line_height() + 12;
    for (std::size_t i = 0; i < kRooms.size(); ++i) {
        int row_y = kPadY + 60 + static_cast<int>(i) * line_h;
        SDL_Color col = ui.theme().text;
        if (static_cast<int>(i) == s.room) {
            SDL_SetRenderDrawColor(ui.renderer(),
                                   ui.theme().highlight.r,
                                   ui.theme().highlight.g,
                                   ui.theme().highlight.b, 200);
            SDL_Rect hl{right_x + 8, row_y - 4, right_w - 16, line_h};
            SDL_RenderFillRect(ui.renderer(), &hl);
            col = SDL_Color{255, 255, 255, 255};
        }
        char buf[120];
        std::snprintf(buf, sizeof(buf), "  %c)  %-16s %lld 金 / 週 (HP +%d)",
                      static_cast<char>('A' + i),
                      kRooms[i].name,
                      kRooms[i].price,
                      kRooms[i].hp_per_week);
        render::draw_text(ui.renderer(), ui.body_font(), buf,
                          right_x + 14, row_y, col);
    }

    if (!state.message_log.empty()) {
        render::draw_text(ui.renderer(), ui.body_font(),
                          std::string("» ") + state.message_log.back(),
                          kPadX + 8, kPadY + 540, ui.theme().accent);
    }

    ui.draw_status_bar("Tab 投宿者  ↑↓ 房間  Enter 投宿  ESC 離開");
    ui.present();
}

void draw_temple(State& state, const render::UI& ui, const TempleUI& s) {
    ui.clear();
    ui.draw_title_bar("坎特神殿");

    const int left_w = 380;
    ui.draw_frame(kPadX, kPadY, left_w, 520);
    render::draw_text(ui.renderer(), ui.body_font(), "病患",
                      kPadX + 14, kPadY + 14, ui.theme().accent);
    int yy = kPadY + 14 + ui.body_font().line_height() + 6;
    for (int i = 0; i < state.party.count; ++i) {
        int ri = state.party.roster_index[i];
        if (ri < 0) continue;
        const auto& c = state.roster.chars[ri];
        SDL_Color col = (i == s.member) ? SDL_Color{255,255,255,255} : ui.theme().text;
        if (i == s.member) {
            SDL_SetRenderDrawColor(ui.renderer(),
                                   ui.theme().highlight.r,
                                   ui.theme().highlight.g,
                                   ui.theme().highlight.b, 200);
            SDL_Rect hl{kPadX + 8, yy - 4, left_w - 16, ui.body_font().line_height() + 8};
            SDL_RenderFillRect(ui.renderer(), &hl);
        }
        char line[160];
        std::snprintf(line, sizeof(line), "%d. %-10s  HP %d/%d  狀態:%s",
                      i + 1, c.name.c_str(),
                      int(c.hp_left), int(c.hp_max),
                      core::status_name(c.status));
        render::draw_text(ui.renderer(), ui.body_font(), line,
                          kPadX + 24, yy, col);
        yy += ui.body_font().line_height() + 8;
    }
    render::draw_text(ui.renderer(), ui.small_font(),
                      "Tab 切換病患", kPadX + 14, kPadY + 520 - 30, ui.theme().dim);

    const int right_x = kPadX + left_w + 24;
    const int right_w = 1280 - right_x - kPadX;
    ui.draw_frame(right_x, kPadY, right_w, 520);
    render::draw_text(ui.renderer(), ui.body_font(),
                      "選擇治療（Enter 確認，自費）",
                      right_x + 14, kPadY + 14, ui.theme().accent);

    const char* labels[] = {
        "[A] 治療 HP        50 金 / 等級",
        "[B] 解毒           75 金 / 等級",
        "[C] 治癒麻痺      100 金 / 等級",
        "[D] 解石化        500 金 / 等級",
        "[E] 復活        1000 金 / 等級",
    };
    const int line_h = ui.body_font().line_height() + 12;
    for (int i = 0; i < TempleUI::Count; ++i) {
        int row_y = kPadY + 60 + i * line_h;
        SDL_Color col = ui.theme().text;
        if (i == s.action) {
            SDL_SetRenderDrawColor(ui.renderer(),
                                   ui.theme().highlight.r,
                                   ui.theme().highlight.g,
                                   ui.theme().highlight.b, 200);
            SDL_Rect hl{right_x + 8, row_y - 4, right_w - 16, line_h};
            SDL_RenderFillRect(ui.renderer(), &hl);
            col = SDL_Color{255, 255, 255, 255};
        }
        render::draw_text(ui.renderer(), ui.body_font(), labels[i],
                          right_x + 14, row_y, col);
    }

    if (!state.message_log.empty()) {
        render::draw_text(ui.renderer(), ui.body_font(),
                          std::string("» ") + state.message_log.back(),
                          kPadX + 8, kPadY + 540, ui.theme().accent);
    }

    ui.draw_status_bar("Tab 病患  ↑↓ 治療  Enter 執行  ESC 離開");
    ui.present();
}

}  // namespace

bool inn_tick(State& state, const SDL_Event* event, const render::UI& ui) {
    auto& s = inn_state();
    if (s.member >= state.party.count) s.member = 0;
    if (event && event->type == SDL_KEYDOWN) {
        SDL_Keycode k = event->key.keysym.sym;
        if (k == SDLK_ESCAPE) { state.change_scene(Scene::Castle); return true; }
        if (k == SDLK_TAB && state.party.count > 0)
            s.member = (s.member + 1) % state.party.count;
        if (k == SDLK_UP)   s.room = (s.room - 1 + int(kRooms.size())) % int(kRooms.size());
        if (k == SDLK_DOWN) s.room = (s.room + 1) % int(kRooms.size());
        if (k == SDLK_RETURN) {
            int ri = state.party.roster_index[s.member];
            if (ri >= 0) {
                auto& c = state.roster.chars[ri];
                const auto& room = kRooms[s.room];
                if (c.gold >= room.price) {
                    c.gold -= room.price;
                    int healed = std::min<int>(room.hp_per_week, c.hp_max - c.hp_left);
                    c.hp_left += static_cast<std::int16_t>(healed);
                    c.age += room.weeks_per_level;
                    state.push_message(
                        c.name + " 投宿 " + room.name + " 一週，HP +" +
                        std::to_string(healed));
                    // Auto level up if XP threshold reached
                    auto needed = core::xp_for_level(c.klass, c.char_level + 1);
                    if (c.experience >= needed) {
                        ++c.char_level;
                        core::recompute_derived(c);
                        c.hp_left = c.hp_max;
                        state.push_message(c.name + " 升級！ 現在等級 " +
                                           std::to_string(c.char_level));
                    }
                } else {
                    state.push_message("** 金幣不足 **");
                }
            }
        }
    }
    draw_inn(state, ui, s);
    return true;
}

bool temple_tick(State& state, const SDL_Event* event, const render::UI& ui) {
    auto& s = temple_state();
    if (s.member >= state.party.count) s.member = 0;
    if (event && event->type == SDL_KEYDOWN) {
        SDL_Keycode k = event->key.keysym.sym;
        if (k == SDLK_ESCAPE) { state.change_scene(Scene::Castle); return true; }
        if (k == SDLK_TAB && state.party.count > 0)
            s.member = (s.member + 1) % state.party.count;
        if (k == SDLK_UP)
            s.action = static_cast<TempleUI::Action>(
                (int(s.action) - 1 + TempleUI::Count) % TempleUI::Count);
        if (k == SDLK_DOWN)
            s.action = static_cast<TempleUI::Action>((int(s.action) + 1) % TempleUI::Count);
        if (k == SDLK_RETURN) {
            int ri = state.party.roster_index[s.member];
            if (ri >= 0) {
                auto& c = state.roster.chars[ri];
                long long cost = 0;
                switch (s.action) {
                    case TempleUI::Heal:           cost = 50LL * c.char_level; break;
                    case TempleUI::CurePoison:     cost = 75LL * c.char_level; break;
                    case TempleUI::CureParalysis:  cost = 100LL * c.char_level; break;
                    case TempleUI::CureStone:      cost = 500LL * c.char_level; break;
                    case TempleUI::Resurrect:      cost = 1000LL * c.char_level; break;
                    default: break;
                }
                if (c.gold < cost) {
                    state.push_message("** 金幣不足 **");
                } else {
                    c.gold -= cost;
                    switch (s.action) {
                        case TempleUI::Heal:
                            c.hp_left = c.hp_max;
                            state.push_message(c.name + " 已完全治癒。");
                            break;
                        case TempleUI::CurePoison:
                            state.push_message(c.name + " 已解除中毒。");
                            break;
                        case TempleUI::CureParalysis:
                            if (c.status == core::Status::Paralyzed) c.status = core::Status::Ok;
                            state.push_message(c.name + " 已解除麻痺。");
                            break;
                        case TempleUI::CureStone:
                            if (c.status == core::Status::Stoned) c.status = core::Status::Ok;
                            state.push_message(c.name + " 已解除石化。");
                            break;
                        case TempleUI::Resurrect:
                            if (c.status == core::Status::Dead ||
                                c.status == core::Status::Ashes) {
                                c.status = core::Status::Ok;
                                c.hp_left = 1;
                                c.age += 52;  // age one year per resurrect
                                state.push_message(c.name + " 已復活（年齡 +1 歲）。");
                            } else {
                                state.push_message(c.name + " 並未死亡。");
                            }
                            break;
                        default: break;
                    }
                }
            }
        }
    }
    draw_temple(state, ui, s);
    return true;
}

}  // namespace wiz::game
