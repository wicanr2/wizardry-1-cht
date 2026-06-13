#include "game/shop.h"

#include <SDL.h>

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

#include "data/items_db.h"

namespace wiz::game {

namespace {

constexpr int kPadX = 80;
constexpr int kPadY = 110;

struct ShopUI {
    int cursor = 0;
    int scroll = 0;
    int active_member = 0;  // who's shopping
};

ShopUI& ui_state() {
    static ShopUI s;
    return s;
}

void draw_shop(State& state, const render::UI& ui, const ShopUI& s) {
    ui.clear();
    ui.draw_title_bar("波塔克貿易站");

    // Left: party purse / member selector
    const int left_w = 380;
    ui.draw_frame(kPadX, kPadY, left_w, 520);
    render::draw_text(ui.renderer(), ui.body_font(), "顧客",
                      kPadX + 14, kPadY + 14, ui.theme().accent);
    int yy = kPadY + 14 + ui.body_font().line_height() + 6;
    for (int i = 0; i < state.party.count; ++i) {
        int ri = state.party.roster_index[i];
        if (ri < 0) continue;
        const auto& c = state.roster.chars[ri];
        char line[120];
        std::snprintf(line, sizeof(line), "%d. %-12s  金:%lld",
                      i + 1, c.name.c_str(), (long long)c.gold);
        SDL_Color col = (i == s.active_member)
                            ? SDL_Color{255, 255, 255, 255}
                            : ui.theme().text;
        if (i == s.active_member) {
            SDL_SetRenderDrawColor(ui.renderer(),
                                   ui.theme().highlight.r,
                                   ui.theme().highlight.g,
                                   ui.theme().highlight.b, 200);
            SDL_Rect hl{kPadX + 8, yy - 4, left_w - 16, ui.body_font().line_height() + 8};
            SDL_RenderFillRect(ui.renderer(), &hl);
        }
        render::draw_text(ui.renderer(), ui.body_font(), line,
                          kPadX + 24, yy, col);
        yy += ui.body_font().line_height() + 8;
    }
    render::draw_text(ui.renderer(), ui.small_font(),
                      "Tab 切換顧客",
                      kPadX + 14, kPadY + 520 - 30, ui.theme().dim);

    // Right: item list
    const int right_x = kPadX + left_w + 24;
    const int right_w = 1280 - right_x - kPadX;
    ui.draw_frame(right_x, kPadY, right_w, 520);
    render::draw_text(ui.renderer(), ui.body_font(),
                      "商品（[B] 購買，↑↓ 選擇，ESC 離開）",
                      right_x + 14, kPadY + 14, ui.theme().accent);

    const auto& items = data::items();
    const int line_h = ui.body_font().line_height() + 4;
    const int rows = (520 - 60) / line_h;
    int first = s.scroll;
    int last = std::min<int>(first + rows, static_cast<int>(items.size()));

    int row_y = kPadY + 50;
    for (int i = first; i < last; ++i) {
        const auto& it = items[i];
        SDL_Color col = ui.theme().text;
        if (i == s.cursor) {
            SDL_SetRenderDrawColor(ui.renderer(),
                                   ui.theme().highlight.r,
                                   ui.theme().highlight.g,
                                   ui.theme().highlight.b, 200);
            SDL_Rect hl{right_x + 8, row_y - 4, right_w - 16, line_h};
            SDL_RenderFillRect(ui.renderer(), &hl);
            col = SDL_Color{255, 255, 255, 255};
        }
        char buf[200];
        std::snprintf(buf, sizeof(buf), "  %-18s  %-8s  %lld 金",
                      it.name_zh.empty() ? it.name_en.c_str()
                                         : it.name_zh.c_str(),
                      it.kind.c_str(),
                      (long long)it.price);
        render::draw_text(ui.renderer(), ui.body_font(), buf,
                          right_x + 14, row_y, col);
        row_y += line_h;
    }

    // Bottom: latest message strip
    if (!state.message_log.empty()) {
        const auto& last = state.message_log.back();
        render::draw_text(ui.renderer(), ui.body_font(),
                          std::string("» ") + last,
                          kPadX + 8, kPadY + 540, ui.theme().accent);
    }

    ui.draw_status_bar("Tab 顧客  ↑↓ 選物  B 買  S 賣  I 鑑定  ESC 離開");
    ui.present();
}

}  // namespace

bool shop_tick(State& state, const SDL_Event* event, const render::UI& ui) {
    auto& s = ui_state();
    const auto& items = data::items();

    if (s.active_member >= state.party.count) s.active_member = 0;
    if (s.cursor >= static_cast<int>(items.size())) s.cursor = 0;

    if (event && event->type == SDL_KEYDOWN) {
        SDL_Keycode k = event->key.keysym.sym;
        if (k == SDLK_ESCAPE) {
            state.change_scene(Scene::Castle);
            return true;
        }
        if (k == SDLK_TAB && state.party.count > 0) {
            s.active_member = (s.active_member + 1) % state.party.count;
        }
        if (k == SDLK_UP && !items.empty()) {
            s.cursor = (s.cursor - 1 + items.size()) % items.size();
            if (s.cursor < s.scroll) s.scroll = s.cursor;
        }
        if (k == SDLK_DOWN && !items.empty()) {
            s.cursor = (s.cursor + 1) % items.size();
            const int rows = (520 - 60) / (ui.body_font().line_height() + 4);
            if (s.cursor >= s.scroll + rows) s.scroll = s.cursor - rows + 1;
            if (s.cursor == 0) s.scroll = 0;
        }
        if (k == SDLK_b && !items.empty()) {
            const auto& it = items[s.cursor];
            int ri = state.party.roster_index[s.active_member];
            if (ri >= 0) {
                auto& c = state.roster.chars[ri];
                // v1.25.2 — Buy must actually add the item to the buyer's
                // inventory. Find first empty slot. Items bought from
                // Boltac arrive identified and uncursed.
                int empty_slot = -1;
                for (int j = 0; j < int(c.inventory.size()); ++j) {
                    if (c.inventory[j].item_id < 0) { empty_slot = j; break; }
                }
                if (empty_slot < 0) {
                    state.push_message("** 背包已滿 **");
                } else if (c.gold < it.price) {
                    state.push_message("** 金幣不足 **");
                } else {
                    c.gold -= it.price;
                    c.inventory[empty_slot].item_id = static_cast<std::int16_t>(it.id);
                    c.inventory[empty_slot].identified = true;
                    c.inventory[empty_slot].cursed = false;
                    c.inventory[empty_slot].equipped = false;
                    state.push_message(
                        c.name + " 購買：" +
                        (it.name_zh.empty() ? it.name_en : it.name_zh) +
                        "（-" + std::to_string(it.price) + " 金）");
                }
            }
        }
        // v1.25.2 — Sell: half-price for first non-empty, non-cursed slot.
        // Boltac refuses to take cursed gear (player must use temple Uncurse).
        if (k == SDLK_s) {
            int ri = state.party.roster_index[s.active_member];
            if (ri >= 0) {
                auto& c = state.roster.chars[ri];
                int sold = -1;
                for (int j = 0; j < int(c.inventory.size()); ++j) {
                    auto& slot = c.inventory[j];
                    if (slot.item_id >= 0 && !slot.cursed) {
                        sold = j;
                        break;
                    }
                }
                if (sold < 0) {
                    state.push_message("** 沒有可賣的道具（詛咒物先去神殿解咒）**");
                } else {
                    int item_id = c.inventory[sold].item_id;
                    long long price = 0;
                    std::string name_show = "(?)";
                    for (const auto& it : items) {
                        if (it.id == item_id) {
                            price = it.price / 2;
                            name_show = it.name_zh.empty() ? it.name_en : it.name_zh;
                            break;
                        }
                    }
                    c.gold += price;
                    c.inventory[sold] = core::ItemSlot{};  // clear
                    state.push_message(
                        c.name + " 賣出：" + name_show +
                        "（+" + std::to_string(price) + " 金，半價）");
                }
            }
        }
        // v1.25.2 — Identify: first unidentified slot, 100 gp/item.
        if (k == SDLK_i) {
            int ri = state.party.roster_index[s.active_member];
            if (ri >= 0) {
                auto& c = state.roster.chars[ri];
                int unidx = -1;
                for (int j = 0; j < int(c.inventory.size()); ++j) {
                    if (c.inventory[j].item_id >= 0 &&
                        !c.inventory[j].identified) {
                        unidx = j; break;
                    }
                }
                const long long cost = 100;
                if (unidx < 0) {
                    state.push_message("** 沒有未鑑定道具 **");
                } else if (c.gold < cost) {
                    state.push_message("** 金幣不足 100 **");
                } else {
                    c.gold -= cost;
                    c.inventory[unidx].identified = true;
                    std::string name_show = "(?)";
                    for (const auto& it : items) {
                        if (it.id == c.inventory[unidx].item_id) {
                            name_show = it.name_zh.empty() ? it.name_en : it.name_zh;
                            break;
                        }
                    }
                    state.push_message(
                        c.name + " 鑑定：" + name_show +
                        "（-" + std::to_string(cost) + " 金）");
                }
            }
        }
    }

    draw_shop(state, ui, s);
    return true;
}

}  // namespace wiz::game
