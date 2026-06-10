#include "game/camp.h"

#include <SDL.h>

#include <sys/stat.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "core/rng.h"
#include "game/class_change.h"
#include "save/gamesave.h"

namespace wiz::game {

namespace {

constexpr int kPadX = 80;
constexpr int kPadY = 110;

enum CampOption { Save, Inspect, Reorder, CastSpell, ChangeClass, Back, ToTitle, Count };
enum class SaveSlot { None = 0, Picking };  // sub-mode markers

struct CampUI {
    int cursor = 0;
    int inspect_idx = 0;
    bool inspecting = false;

    // Spell sub-mode
    bool picking_spell = false;
    int spell_cursor = 0;
    int spell_target = 0;

    // Save slot picker
    bool picking_save_slot = false;
    int save_slot_cursor = 0;  // 0..kNumSlots-1 → slot 1..kNumSlots

    // Class-change sub-mode
    bool picking_class_change = false;
    int class_change_member = 0;     // party slot 0..count-1
    int class_change_target = 0;     // Klass enum value
};

// Camp-castable spells (subset of all 51, only ones meaningful outside combat).
constexpr std::array<const char*, 11> kCampSpells = {{
    "DIOS",      // self heal
    "DIAL",      // self heal more
    "DIALMA",    // self heal lots
    "MADI",      // full heal
    "MILWA",     // brief light
    "LOMILWA",   // long light
    "CALFO",     // identify trap (on chest)
    "LATUMOFIS", // cure poison
    "DIALKO",    // cure paralysis/sleep
    "DUMAPIC",   // show location
    "DI",        // resurrect (on dead)
}};

CampUI& ui_state() { static CampUI s; return s; }

const char* option_label(int o) {
    switch (o) {
        case Save:      return "[S] 存檔";
        case Inspect:   return "[I] 檢視角色";
        case Reorder:   return "[R] 重排隊伍";
        case CastSpell: return "[C] 施法（治療/照明/...）";
        case ChangeClass: return "[X] 轉職";
        case Back:      return "[ESC] 回到迷宮";
        case ToTitle:   return "[Q] 回到標題";
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

    if (s.picking_save_slot) {
        const int rx = right_x + 14;
        int ry = kPadY + 14;
        render::draw_text(ui.renderer(), ui.body_font(),
                          "選擇存檔槽（↑↓ 或 1-5 鍵，Enter 確認，ESC 取消）",
                          rx, ry, ui.theme().accent);
        ry += ui.body_font().line_height() + 16;
        for (int i = 0; i < kNumSlots; ++i) {
            std::string p = save_path_for_slot(i + 1);
            const char* status = "（空）";
            // Quick stat check
            struct stat st;
            if (stat(p.c_str(), &st) == 0) status = "（已用）";
            char line[200];
            std::snprintf(line, sizeof(line), "%s Slot %d  %s",
                          i == s.save_slot_cursor ? "▸" : "  ",
                          i + 1, status);
            SDL_Color col = (i == s.save_slot_cursor)
                              ? SDL_Color{255, 255, 0, 255}
                              : ui.theme().text;
            render::draw_text(ui.renderer(), ui.body_font(), line,
                              rx, ry + i * (ui.body_font().line_height() + 8), col);
        }
    } else if (s.picking_class_change) {
        const int rx = right_x + 14;
        int ry = kPadY + 14;
        render::draw_text(ui.renderer(), ui.body_font(),
                          "轉職（←→ 選成員 / ↑↓ 選新職 / Enter 確認 / ESC 取消）",
                          rx, ry, ui.theme().accent);
        ry += ui.body_font().line_height() + 10;
        char buf[200];
        int ri = (s.class_change_member >= 0 &&
                  s.class_change_member < state.party.count)
                     ? state.party.roster_index[s.class_change_member] : -1;
        if (ri >= 0) {
            const auto& c = state.roster.chars[ri];
            std::snprintf(buf, sizeof(buf), "成員：%d. %s (%s Lv%d)",
                          s.class_change_member + 1, c.name.c_str(),
                          core::klass_name(c.klass), int(c.char_level));
        } else {
            std::snprintf(buf, sizeof(buf), "成員：(無)");
        }
        render::draw_text(ui.renderer(), ui.body_font(), buf, rx, ry, ui.theme().text);
        ry += ui.body_font().line_height() + 14;
        const int line_h = ui.body_font().line_height() + 6;
        for (int i = 0; i < static_cast<int>(core::Klass::Count); ++i) {
            auto klass = static_cast<core::Klass>(i);
            std::string reason;
            if (ri >= 0) reason = can_change_to(state.roster.chars[ri], klass);
            const char* status = reason.empty() ? "可" : reason.c_str();
            SDL_Color col = (i == s.class_change_target)
                                ? SDL_Color{255, 255, 0, 255}
                                : (reason.empty() ? ui.theme().text
                                                  : SDL_Color{140, 140, 140, 255});
            std::snprintf(buf, sizeof(buf), "%s%-12s [%s]",
                          i == s.class_change_target ? "▸ " : "  ",
                          core::klass_name(klass), status);
            render::draw_text(ui.renderer(), ui.body_font(), buf,
                              rx, ry + i * line_h, col);
        }
    } else if (s.picking_spell) {
        const int rx = right_x + 14;
        int ry = kPadY + 14;
        render::draw_text(ui.renderer(), ui.body_font(), "施法（↑↓ 法術 / ←→ 對象 / Enter 施 / ESC 取消）",
                          rx, ry, ui.theme().accent);
        ry += ui.body_font().line_height() + 10;
        // Target
        int target_ri = (s.spell_target >= 0 && s.spell_target < state.party.count)
                            ? state.party.roster_index[s.spell_target] : -1;
        char buf[160];
        if (target_ri >= 0) {
            const auto& c = state.roster.chars[target_ri];
            std::snprintf(buf, sizeof(buf), "對象：%d. %s (HP %d/%d)",
                          s.spell_target + 1, c.name.c_str(),
                          int(c.hp_left), int(c.hp_max));
        } else {
            std::snprintf(buf, sizeof(buf), "對象：(無)");
        }
        render::draw_text(ui.renderer(), ui.body_font(), buf, rx, ry, ui.theme().text);
        ry += ui.body_font().line_height() + 16;

        // Spell list
        const int line_h = ui.body_font().line_height() + 6;
        for (std::size_t i = 0; i < kCampSpells.size(); ++i) {
            SDL_Color col = (static_cast<int>(i) == s.spell_cursor)
                                ? SDL_Color{255, 255, 0, 255}
                                : ui.theme().text;
            std::snprintf(buf, sizeof(buf), "%s%s",
                          static_cast<int>(i) == s.spell_cursor ? "▸ " : "  ",
                          kCampSpells[i]);
            render::draw_text(ui.renderer(), ui.body_font(), buf,
                              rx, ry + static_cast<int>(i) * line_h, col);
        }
    } else if (s.inspecting && s.inspect_idx >= 0 && s.inspect_idx < state.party.count) {
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

std::string cast_camp_spell(State& state, std::string_view spell_name,
                            int target_member) {
    using namespace wiz::core;
    auto& rng = global_rng();

    auto target_ri = [&]() -> int {
        if (target_member < 0 || target_member >= state.party.count) return -1;
        return state.party.roster_index[target_member];
    };
    auto target_char = [&]() -> Character* {
        int ri = target_ri();
        if (ri < 0) return nullptr;
        return &state.roster.chars[ri];
    };

    char buf[200];
    if (spell_name == "DIOS") {
        auto* c = target_char();
        if (!c) return "** 目標無效 **";
        int amt = rng.dice(1, 8);
        int actual = std::min<int>(c->hp_max - c->hp_left, amt);
        c->hp_left = static_cast<std::int16_t>(c->hp_left + actual);
        std::snprintf(buf, sizeof(buf), "DIOS：%s +%d HP", c->name.c_str(), actual);
        return buf;
    }
    if (spell_name == "DIAL") {
        auto* c = target_char();
        if (!c) return "** 目標無效 **";
        int amt = rng.dice(2, 8);
        int actual = std::min<int>(c->hp_max - c->hp_left, amt);
        c->hp_left = static_cast<std::int16_t>(c->hp_left + actual);
        std::snprintf(buf, sizeof(buf), "DIAL：%s +%d HP", c->name.c_str(), actual);
        return buf;
    }
    if (spell_name == "DIALMA") {
        auto* c = target_char();
        if (!c) return "** 目標無效 **";
        int amt = rng.dice(3, 8);
        int actual = std::min<int>(c->hp_max - c->hp_left, amt);
        c->hp_left = static_cast<std::int16_t>(c->hp_left + actual);
        std::snprintf(buf, sizeof(buf), "DIALMA：%s +%d HP", c->name.c_str(), actual);
        return buf;
    }
    if (spell_name == "MADI") {
        // Heal all
        int healed = 0;
        for (int i = 0; i < state.party.count; ++i) {
            int ri = state.party.roster_index[i];
            if (ri < 0) continue;
            auto& c = state.roster.chars[ri];
            int missing = c.hp_max - c.hp_left;
            healed += missing;
            c.hp_left = c.hp_max;
        }
        std::snprintf(buf, sizeof(buf), "MADI：全隊回滿 (+%d HP)", healed);
        return buf;
    }
    if (spell_name == "DUMAPIC") {
        const char* face = "北";
        switch (state.camera.facing) {
            case render::Facing::North: face = "北"; break;
            case render::Facing::East:  face = "東"; break;
            case render::Facing::South: face = "南"; break;
            case render::Facing::West:  face = "西"; break;
        }
        std::snprintf(buf, sizeof(buf),
                      "DUMAPIC：B%dF X:%d Y:%d 面向%s",
                      state.camera.level, state.camera.x, state.camera.y, face);
        return buf;
    }
    if (spell_name == "MILWA" || spell_name == "LOMILWA") {
        // Reveal all visited + adjacent cells on auto-map
        for (int y = 0; y < core::MazeLevel::kSize; ++y) {
            for (int x = 0; x < core::MazeLevel::kSize; ++x) {
                if (state.maze.visited[y][x]) {
                    if (y > 0) state.maze.visited[y-1][x] = true;
                    if (y < core::MazeLevel::kSize - 1) state.maze.visited[y+1][x] = true;
                    if (x > 0) state.maze.visited[y][x-1] = true;
                    if (x < core::MazeLevel::kSize - 1) state.maze.visited[y][x+1] = true;
                }
            }
        }
        return spell_name == "MILWA" ? "MILWA：短時照明，已揭露鄰格"
                                     : "LOMILWA：長時照明，已揭露鄰格";
    }
    if (spell_name == "LATUMOFIS") {
        auto* c = target_char();
        if (!c) return "** 目標無效 **";
        if (c->status == Status::Poisoned) {
            c->status = Status::Ok;
            c->poison_strength = 0;
            std::snprintf(buf, sizeof(buf),
                          "LATUMOFIS：%s 中毒已解除", c->name.c_str());
        } else {
            std::snprintf(buf, sizeof(buf),
                          "LATUMOFIS：%s 並未中毒", c->name.c_str());
        }
        return buf;
    }
    if (spell_name == "DIALKO") {
        auto* c = target_char();
        if (!c) return "** 目標無效 **";
        if (c->status == Status::Paralyzed || c->status == Status::Asleep) {
            c->status = Status::Ok;
            std::snprintf(buf, sizeof(buf), "DIALKO：%s 麻痺/沉睡已解除", c->name.c_str());
        } else {
            std::snprintf(buf, sizeof(buf), "DIALKO：%s 並未麻痺", c->name.c_str());
        }
        return buf;
    }
    if (spell_name == "CALFO") {
        return "CALFO：附近無寶箱可鑑定";
    }
    if (spell_name == "DI") {
        auto* c = target_char();
        if (!c) return "** 目標無效 **";
        if (c->status == Status::Dead) {
            // 70 + level % success
            int chance = 70 + c->char_level * 2;
            if (rng.range(1, 100) <= chance) {
                c->status = Status::Ok;
                c->hp_left = 1;
                c->age += 52;
                std::snprintf(buf, sizeof(buf), "DI：%s 復活！年齡 +1 歲", c->name.c_str());
            } else {
                c->status = Status::Ashes;
                std::snprintf(buf, sizeof(buf), "DI：%s 復活失敗，化為灰燼…", c->name.c_str());
            }
        } else {
            std::snprintf(buf, sizeof(buf), "DI：%s 並未死亡", c->name.c_str());
        }
        return buf;
    }
    return "** 法術尚未實作 **";
}

std::string save_path_for_slot(int slot) {
    if (slot < 1) slot = 1;
    if (slot > kNumSlots) slot = kNumSlots;
    char fname[64];
    std::snprintf(fname, sizeof(fname), "/save_%d.json", slot);
    const char* home = std::getenv("HOME");
    if (home && *home) return std::string(home) + "/.config/wizardry-cht" + fname;
    return std::string(".") + fname;
}

std::string default_save_path() { return save_path_for_slot(1); }

bool camp_tick(State& state, const SDL_Event* event, const render::UI& ui) {
    auto& s = ui_state();

    if (event && event->type == SDL_KEYDOWN) {
        SDL_Keycode k = event->key.keysym.sym;

        if (s.picking_save_slot) {
            if (k == SDLK_ESCAPE) { s.picking_save_slot = false; }
            else if (k == SDLK_UP) {
                s.save_slot_cursor = (s.save_slot_cursor - 1 + kNumSlots) % kNumSlots;
            } else if (k == SDLK_DOWN) {
                s.save_slot_cursor = (s.save_slot_cursor + 1) % kNumSlots;
            } else if (k >= SDLK_1 && k <= SDLK_5) {
                s.save_slot_cursor = k - SDLK_1;
            }
            if (k == SDLK_RETURN || k == SDLK_SPACE ||
                (k >= SDLK_1 && k <= SDLK_5)) {
                std::string p = save_path_for_slot(s.save_slot_cursor + 1);
                auto slash = p.find_last_of('/');
                if (slash != std::string::npos) {
                    std::string cmd = "mkdir -p '" + p.substr(0, slash) + "'";
                    (void)std::system(cmd.c_str());
                }
                if (save::save_game(state, p)) {
                    char buf[200];
                    std::snprintf(buf, sizeof(buf),
                                  "✦ 已存到 Slot %d：%s",
                                  s.save_slot_cursor + 1, p.c_str());
                    state.push_message(buf);
                } else {
                    state.push_message("** 存檔失敗 **");
                }
                s.picking_save_slot = false;
            }
        } else if (s.picking_spell) {
            int n = static_cast<int>(kCampSpells.size());
            int pc = static_cast<int>(state.party.count);
            if (k == SDLK_ESCAPE) {
                s.picking_spell = false;
            } else if (k == SDLK_UP) {
                s.spell_cursor = (s.spell_cursor - 1 + n) % n;
            } else if (k == SDLK_DOWN) {
                s.spell_cursor = (s.spell_cursor + 1) % n;
            } else if (k == SDLK_LEFT && pc > 0) {
                s.spell_target = (s.spell_target - 1 + pc) % pc;
            } else if (k == SDLK_RIGHT && pc > 0) {
                s.spell_target = (s.spell_target + 1) % pc;
            } else if (k == SDLK_RETURN) {
                auto msg = cast_camp_spell(state, kCampSpells[s.spell_cursor],
                                           s.spell_target);
                state.push_message(std::move(msg));
                s.picking_spell = false;
            }
        } else if (s.picking_class_change) {
            int pc = static_cast<int>(state.party.count);
            int nk = static_cast<int>(core::Klass::Count);
            if (k == SDLK_ESCAPE) {
                s.picking_class_change = false;
            } else if (k == SDLK_LEFT && pc > 0) {
                s.class_change_member = (s.class_change_member - 1 + pc) % pc;
            } else if (k == SDLK_RIGHT && pc > 0) {
                s.class_change_member = (s.class_change_member + 1) % pc;
            } else if (k == SDLK_UP) {
                s.class_change_target = (s.class_change_target - 1 + nk) % nk;
            } else if (k == SDLK_DOWN) {
                s.class_change_target = (s.class_change_target + 1) % nk;
            } else if (k == SDLK_RETURN) {
                int ri = state.party.roster_index[s.class_change_member];
                if (ri >= 0) {
                    auto& c = state.roster.chars[ri];
                    auto target = static_cast<core::Klass>(s.class_change_target);
                    std::string reason = can_change_to(c, target);
                    if (!reason.empty()) {
                        state.push_message(c.name + " 無法轉職：" + reason);
                    } else {
                        auto new_name = apply_class_change(c, target);
                        char buf[200];
                        std::snprintf(buf, sizeof(buf),
                                      "✦ %s 轉職為 %s（等級重置為 1，HP %d）",
                                      c.name.c_str(), std::string(new_name).c_str(),
                                      int(c.hp_max));
                        state.push_message(buf);
                    }
                }
                s.picking_class_change = false;
            }
        } else if (s.inspecting) {
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
                    case Save:
                        s.picking_save_slot = true;
                        s.save_slot_cursor = 0;
                        break;
                    case Inspect:
                        s.inspecting = true;
                        s.inspect_idx = 0;
                        break;
                    case CastSpell:
                        s.picking_spell = true;
                        s.spell_cursor = 0;
                        s.spell_target = 0;
                        break;
                    case ChangeClass:
                        if (state.party.count == 0) {
                            state.push_message("** 隊伍為空 **");
                        } else {
                            s.picking_class_change = true;
                            s.class_change_member = 0;
                            s.class_change_target = 0;
                        }
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
            else if (k == SDLK_c) run(CastSpell);
            else if (k == SDLK_x) run(ChangeClass);
            else if (k == SDLK_q) run(ToTitle);
        }
    }

    draw_camp(state, ui, s);
    return true;
}

}  // namespace wiz::game
