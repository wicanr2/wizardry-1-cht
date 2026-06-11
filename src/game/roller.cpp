#include "game/roller.h"

#include <SDL.h>

#include <array>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "core/rng.h"
#include "core/rules.h"
#include "i18n/tr.h"

namespace wiz::game {

namespace {

constexpr int kPadX = 80;
constexpr int kPadY = 110;

const std::array<const char*, 5> kRaceLabels = {
    "人類", "精靈", "矮人", "侏儒", "霍比特"
};
const std::array<const char*, 3> kAlignLabels = {"善良", "中立", "邪惡"};
const std::array<const char*, 8> kKlassLabels = {
    "戰士", "魔法師", "牧師", "盜賊", "主教", "武士", "領主", "忍者"
};
const std::array<const char*, 6> kAttrLabels = {
    "力量", "智力", "信仰", "體力", "敏捷", "幸運"
};

// Class lore from Sir-tech Manual p14-15 (Ultimate Wizardry Archives, 1998
// Interplay edition). Used as tooltip in the class selection screen.
const std::array<const char*, 8> kKlassLore = {
    "戰士。基本的武人，HP 高、可用任何盔甲與武器，戰鬥的中堅。",
    "魔法師。法術施展者，HP 低、只能用匕首與法杖，不可穿盔甲（除袍）；能施魔法系法術。",
    "牧師。聖職者，HP 中等；不可用刀劍只能用棍棒，可施神聖法術，可驅散不死生物。",
    "盜賊。能用匕首或短劍、皮甲與小盾；擅長解除寶箱陷阱，不可為善良陣營。",
    "主教。法師 + 牧師複合，可施兩系法術、能鑑定物品；魔法成長較慢，不可為中立。",
    "武士。戰士的進化版，HP 高於戰士初期；4 級開始學魔法系法術。武士守 Bushido，不可邪惡。",
    "領主。戰士 + 牧師複合，4 級開始學神聖法術；HP 與盔甲沿用戰士。必須為善良陣營。",
    "忍者。超人戰士，赤手空拳極強（每擊有機率秒殺！），裝備越少越強。等級越高 AC 越好。必須為邪惡陣營。",
};

const std::array<const char*, 5> kRaceLore = {
    "人類。樣樣中等，沒有特別弱項，但信仰偏低。",
    "精靈。智力與信仰高，但體質弱；天生的法師體質。",
    "矮人。力量與體質強，喜歡好武器與盔甲，善戰。",
    "侏儒。信仰與敏捷高，據說是地下祈禱練出來的；理想的牧師人選。",
    "霍比特。敏捷高、運氣非常好；訓練得當會成為頂尖盜賊。",
};

std::uint8_t* attr_ref(core::Attributes& a, int idx) {
    switch (idx) {
        case 0: return &a.strength;
        case 1: return &a.iq;
        case 2: return &a.piety;
        case 3: return &a.vitality;
        case 4: return &a.agility;
        case 5: return &a.luck;
    }
    return &a.strength;
}

void draw_step_header(const render::UI& ui, std::string_view title,
                      std::string_view hint) {
    ui.clear();
    ui.draw_title_bar(title);
    ui.draw_status_bar(hint);
}

void draw_name_screen(const RollerState& r, const render::UI& ui) {
    draw_step_header(ui, "建立角色 — 輸入名字",
                     "輸入名字後按 Enter 確定，ESC 取消");
    ui.draw_frame(kPadX, kPadY, 1280 - 2 * kPadX, 200);
    render::draw_text(ui.renderer(), ui.body_font(), "請輸入角色名字 (1-15 字)：",
                      kPadX + 24, kPadY + 30, ui.theme().accent);
    std::string display = "  " + r.name_buf + "_";
    render::draw_text(ui.renderer(), ui.title_font(), display,
                      kPadX + 24, kPadY + 80, ui.theme().text);
    render::draw_text(ui.renderer(), ui.small_font(),
                      "範例：ALOZAR、勇者小張、Werdna",
                      kPadX + 24, kPadY + 150, ui.theme().dim);
    ui.present();
}

void draw_pick_list(const RollerState& r, const render::UI& ui,
                    std::string_view title,
                    const std::vector<std::string>& options,
                    const std::vector<const char*>& lore = {}) {
    draw_step_header(ui, title, "↑↓ 選擇   Enter 確認   ESC 取消");
    const int x = kPadX;
    const int y = kPadY;
    const int w = 1280 - 2 * kPadX;
    const int h = 480;
    ui.draw_frame(x, y, w, h);

    const int line = ui.body_font().line_height() + 10;
    const int pad = 30;
    const int list_w = 380;

    for (std::size_t i = 0; i < options.size(); ++i) {
        int yy = y + pad + static_cast<int>(i) * line;
        if (static_cast<int>(i) == r.cursor) {
            SDL_SetRenderDrawColor(ui.renderer(),
                                   ui.theme().highlight.r,
                                   ui.theme().highlight.g,
                                   ui.theme().highlight.b, 220);
            SDL_Rect hl{x + 16, yy - 4, list_w, line};
            SDL_RenderFillRect(ui.renderer(), &hl);
        }
        SDL_Color c = (static_cast<int>(i) == r.cursor)
                          ? SDL_Color{255, 255, 255, 255}
                          : ui.theme().text;
        char label[64];
        std::snprintf(label, sizeof(label), "  %c)  %s",
                      static_cast<char>('A' + i), options[i].c_str());
        render::draw_text(ui.renderer(), ui.body_font(), label,
                          x + 24, yy, c);
    }

    // Lore tooltip panel on the right
    if (!lore.empty() && r.cursor < static_cast<int>(lore.size())) {
        int lore_x = x + list_w + 40;
        int lore_y = y + pad;
        int lore_w = w - list_w - 80;
        render::draw_text(ui.renderer(), ui.body_font(), "說明",
                          lore_x, lore_y, ui.theme().accent);
        lore_y += ui.body_font().line_height() + 10;

        // Word-wrap lore text manually (Chinese chars are full-width)
        std::string text = lore[r.cursor];
        const int max_chars_per_line = (lore_w - 20) / 18;  // ~18px per CJK char
        std::string line_buf;
        int char_count = 0;
        auto flush = [&]() {
            if (!line_buf.empty()) {
                render::draw_text(ui.renderer(), ui.body_font(), line_buf,
                                  lore_x, lore_y, ui.theme().text);
                lore_y += ui.body_font().line_height() + 4;
                line_buf.clear();
                char_count = 0;
            }
        };
        std::size_t i = 0;
        while (i < text.size()) {
            unsigned char ch = static_cast<unsigned char>(text[i]);
            int byte_len = 1;
            int char_width = 1;
            if (ch >= 0xF0) { byte_len = 4; char_width = 2; }
            else if (ch >= 0xE0) { byte_len = 3; char_width = 2; }
            else if (ch >= 0xC0) { byte_len = 2; char_width = 1; }

            if (char_count + char_width > max_chars_per_line || ch == '\n') {
                flush();
                if (ch == '\n') { i += 1; continue; }
            }
            line_buf.append(text, i, byte_len);
            char_count += char_width;
            i += byte_len;
        }
        flush();
    }

    ui.present();
}

void draw_attribute_screen(const RollerState& r, const render::UI& ui) {
    draw_step_header(ui, "建立角色 — 屬性點數",
                     "↑↓ 選屬性   ←→ 加減點   Enter 完成   ESC 取消");
    const int x = kPadX;
    const int y = kPadY;
    const int w = 1280 - 2 * kPadX;
    const int h = 480;
    ui.draw_frame(x, y, w, h);

    char buf[80];
    std::snprintf(buf, sizeof(buf), "種族：%s    陣營：%s",
                  kRaceLabels[static_cast<int>(r.race)],
                  kAlignLabels[static_cast<int>(r.alignment)]);
    render::draw_text(ui.renderer(), ui.body_font(), buf,
                      x + 24, y + 20, ui.theme().accent);

    std::snprintf(buf, sizeof(buf), "剩餘紅利點數：%d", r.bonus_pts);
    render::draw_text(ui.renderer(), ui.body_font(), buf,
                      x + 24, y + 60, ui.theme().accent);

    const core::Attributes& a = r.attr;
    const std::array<int, 6> vals = {
        a.strength, a.iq, a.piety, a.vitality, a.agility, a.luck
    };

    for (int i = 0; i < 6; ++i) {
        int yy = y + 120 + i * 40;
        SDL_Color c = ui.theme().text;
        if (i == r.focused_attr) {
            SDL_SetRenderDrawColor(ui.renderer(),
                                   ui.theme().highlight.r,
                                   ui.theme().highlight.g,
                                   ui.theme().highlight.b, 200);
            SDL_Rect hl{x + 24, yy - 4, 360, 36};
            SDL_RenderFillRect(ui.renderer(), &hl);
            c = SDL_Color{255, 255, 255, 255};
        }
        std::snprintf(buf, sizeof(buf), "  %s : %2d", kAttrLabels[i], vals[i]);
        render::draw_text(ui.renderer(), ui.body_font(), buf,
                          x + 40, yy, c);
    }

    render::draw_text(ui.renderer(), ui.small_font(),
                      "提示：屬性需符合目標職業最低需求，剩餘點數可全為 0",
                      x + 24, y + h - 40, ui.theme().dim);
    ui.present();
}

void draw_class_screen_with_lore(const RollerState& r, const render::UI& ui,
                                 const std::vector<int>& eligible_indices) {
    draw_step_header(ui, "建立角色 — 選擇職業",
                     "↑↓ 選擇   Enter 確認   ESC 取消");
    const int x = kPadX;
    const int y = kPadY;
    const int w = 1280 - 2 * kPadX;
    const int h = 480;
    ui.draw_frame(x, y, w, h);

    render::draw_text(ui.renderer(), ui.body_font(),
                      "可選職業（依屬性與陣營過濾）：",
                      x + 24, y + 20, ui.theme().accent);

    const int line = ui.body_font().line_height() + 10;
    const int list_w = 280;
    if (eligible_indices.empty()) {
        render::draw_text(ui.renderer(), ui.body_font(),
                          "** 沒有符合資格的職業，請按 ESC 重新分配屬性 **",
                          x + 24, y + 80, ui.theme().dim);
    } else {
        for (std::size_t i = 0; i < eligible_indices.size(); ++i) {
            int yy = y + 80 + static_cast<int>(i) * line;
            int ki = eligible_indices[i];
            if (static_cast<int>(i) == r.cursor) {
                SDL_SetRenderDrawColor(ui.renderer(),
                                       ui.theme().highlight.r,
                                       ui.theme().highlight.g,
                                       ui.theme().highlight.b, 200);
                SDL_Rect hl{x + 16, yy - 4, list_w, line};
                SDL_RenderFillRect(ui.renderer(), &hl);
            }
            SDL_Color c = (static_cast<int>(i) == r.cursor)
                              ? SDL_Color{255, 255, 255, 255}
                              : ui.theme().text;
            char buf[64];
            std::snprintf(buf, sizeof(buf), "  %s",
                          kKlassLabels[ki]);
            render::draw_text(ui.renderer(), ui.body_font(), buf,
                              x + 40, yy, c);
        }

        // Lore tooltip for currently focused class
        if (r.cursor < static_cast<int>(eligible_indices.size())) {
            int ki = eligible_indices[r.cursor];
            int lore_x = x + list_w + 40;
            int lore_y = y + 80;
            int lore_w = w - list_w - 80;
            render::draw_text(ui.renderer(), ui.body_font(),
                              std::string("● ") + kKlassLabels[ki],
                              lore_x, lore_y, ui.theme().accent);
            lore_y += ui.body_font().line_height() + 12;

            // Manual word-wrap (same algorithm as draw_pick_list)
            std::string text = kKlassLore[ki];
            const int max_chars = (lore_w - 20) / 18;
            std::string line_buf;
            int char_count = 0;
            auto flush = [&]() {
                if (!line_buf.empty()) {
                    render::draw_text(ui.renderer(), ui.body_font(), line_buf,
                                      lore_x, lore_y, ui.theme().text);
                    lore_y += ui.body_font().line_height() + 4;
                    line_buf.clear();
                    char_count = 0;
                }
            };
            std::size_t bi = 0;
            while (bi < text.size()) {
                unsigned char ch = static_cast<unsigned char>(text[bi]);
                int byte_len = 1, cw = 1;
                if (ch >= 0xF0) { byte_len = 4; cw = 2; }
                else if (ch >= 0xE0) { byte_len = 3; cw = 2; }
                else if (ch >= 0xC0) { byte_len = 2; cw = 1; }
                if (char_count + cw > max_chars) flush();
                line_buf.append(text, bi, byte_len);
                char_count += cw;
                bi += byte_len;
            }
            flush();

            // Show requirements
            lore_y += 8;
            const char* req = "";
            switch (static_cast<core::Klass>(ki)) {
                case core::Klass::Fighter: req = "需求：力量 11+"; break;
                case core::Klass::Mage:    req = "需求：智力 11+"; break;
                case core::Klass::Priest:  req = "需求：信仰 11+ / 非中立"; break;
                case core::Klass::Thief:   req = "需求：敏捷 11+ / 非善良"; break;
                case core::Klass::Bishop:  req = "需求：智力 12+ 信仰 12+ / 非中立"; break;
                case core::Klass::Samurai: req = "需求：力量 15 智力 11 信仰 10 體力 14 敏捷 10 / 非邪惡"; break;
                case core::Klass::Lord:    req = "需求：力 15 智 12 信 12 體 15 敏 14 幸 15 / 必須善良"; break;
                case core::Klass::Ninja:   req = "需求：所有屬性 15+ / 必須邪惡 (v3.2 修正)"; break;
                default: break;
            }
            render::draw_text(ui.renderer(), ui.small_font(), req,
                              lore_x, lore_y, ui.theme().dim);
        }
    }
    ui.present();
}

std::vector<int> compute_eligible(const core::Attributes& a, core::Alignment al) {
    std::vector<int> out;
    for (int k = 0; k < static_cast<int>(core::Klass::Count); ++k) {
        if (core::eligible_for_class(a, al, static_cast<core::Klass>(k))) {
            out.push_back(k);
        }
    }
    return out;
}

}  // namespace

bool roller_tick(RollerState& r, State& state, const SDL_Event* event,
                 const render::UI& ui, core::Character& out) {
    (void)state;

    auto cancel = [&]() {
        state.change_scene(Scene::EdgeOfTown);
        r = RollerState{};
    };

    switch (r.step) {
        case RollerStep::Name: {
            if (event) {
                if (event->type == SDL_KEYDOWN) {
                    SDL_Keycode k = event->key.keysym.sym;
                    if (k == SDLK_ESCAPE) { cancel(); return false; }
                    if (k == SDLK_RETURN || k == SDLK_KP_ENTER) {
                        if (r.name_buf.empty()) {
                            state.push_message("名字不可為空");
                        } else {
                            r.step = RollerStep::Race;
                            r.cursor = 0;
                        }
                    } else if (k == SDLK_BACKSPACE && !r.name_buf.empty()) {
                        // utf-8 safe back-step: drop trailing continuation bytes too
                        while (!r.name_buf.empty() &&
                               (static_cast<unsigned char>(r.name_buf.back()) & 0xC0) == 0x80) {
                            r.name_buf.pop_back();
                        }
                        if (!r.name_buf.empty()) r.name_buf.pop_back();
                    }
                } else if (event->type == SDL_TEXTINPUT) {
                    if (r.name_buf.size() + std::strlen(event->text.text) <= 30) {
                        r.name_buf += event->text.text;
                    }
                }
            }
            draw_name_screen(r, ui);
            return true;
        }

        case RollerStep::Race: {
            std::vector<std::string> opts;
            for (auto s : kRaceLabels) opts.emplace_back(s);
            std::vector<const char*> lore(kRaceLore.begin(), kRaceLore.end());
            if (event && event->type == SDL_KEYDOWN) {
                SDL_Keycode k = event->key.keysym.sym;
                if (k == SDLK_ESCAPE) { cancel(); return false; }
                if (k == SDLK_UP)   r.cursor = (r.cursor - 1 + opts.size()) % opts.size();
                if (k == SDLK_DOWN) r.cursor = (r.cursor + 1) % opts.size();
                if (k == SDLK_RETURN) {
                    r.race = static_cast<core::Race>(r.cursor);
                    r.step = RollerStep::Alignment;
                    r.cursor = 0;
                }
            }
            draw_pick_list(r, ui, "建立角色 — 選擇種族", opts, lore);
            return true;
        }

        case RollerStep::Alignment: {
            std::vector<std::string> opts;
            for (auto s : kAlignLabels) opts.emplace_back(s);
            if (event && event->type == SDL_KEYDOWN) {
                SDL_Keycode k = event->key.keysym.sym;
                if (k == SDLK_ESCAPE) { cancel(); return false; }
                if (k == SDLK_UP)   r.cursor = (r.cursor - 1 + opts.size()) % opts.size();
                if (k == SDLK_DOWN) r.cursor = (r.cursor + 1) % opts.size();
                if (k == SDLK_RETURN) {
                    r.alignment = static_cast<core::Alignment>(r.cursor);
                    r.step = RollerStep::AttributeRoll;
                    r.cursor = 0;
                }
            }
            draw_pick_list(r, ui, "建立角色 — 選擇陣營", opts);
            return true;
        }

        case RollerStep::AttributeRoll: {
            // Auto-roll on entry
            r.attr = core::base_attributes_for(r.race);
            r.bonus_pts = core::roll_bonus_points();
            r.step = RollerStep::DistributeBonus;
            r.focused_attr = 0;
            state.push_message(std::string("獲得紅利點數：") + std::to_string(r.bonus_pts));
            return true;
        }

        case RollerStep::DistributeBonus: {
            if (event && event->type == SDL_KEYDOWN) {
                SDL_Keycode k = event->key.keysym.sym;
                if (k == SDLK_ESCAPE) { cancel(); return false; }
                if (k == SDLK_UP)   r.focused_attr = (r.focused_attr - 1 + 6) % 6;
                if (k == SDLK_DOWN) r.focused_attr = (r.focused_attr + 1) % 6;
                if (k == SDLK_RIGHT && r.bonus_pts > 0) {
                    auto* p = attr_ref(r.attr, r.focused_attr);
                    if (*p < 18) { ++(*p); --r.bonus_pts; }
                }
                if (k == SDLK_LEFT) {
                    auto* p = attr_ref(r.attr, r.focused_attr);
                    auto base = core::base_attributes_for(r.race);
                    int min_val = *attr_ref(base, r.focused_attr);
                    if (static_cast<int>(*p) > min_val) {
                        --(*p);
                        ++r.bonus_pts;
                    }
                }
                if (k == SDLK_RETURN) {
                    r.step = RollerStep::PickClass;
                    r.cursor = 0;
                }
            }
            draw_attribute_screen(r, ui);
            return true;
        }

        case RollerStep::PickClass: {
            auto eligible = compute_eligible(r.attr, r.alignment);
            if (event && event->type == SDL_KEYDOWN) {
                SDL_Keycode k = event->key.keysym.sym;
                if (k == SDLK_ESCAPE) {
                    r.step = RollerStep::DistributeBonus;
                    return true;
                }
                if (k == SDLK_UP && !eligible.empty()) {
                    r.cursor = (r.cursor - 1 + eligible.size()) % eligible.size();
                }
                if (k == SDLK_DOWN && !eligible.empty()) {
                    r.cursor = (r.cursor + 1) % eligible.size();
                }
                if (k == SDLK_RETURN && !eligible.empty()) {
                    r.klass = static_cast<core::Klass>(eligible[r.cursor]);
                    r.step = RollerStep::Done;
                }
            }
            draw_class_screen_with_lore(r, ui, eligible);
            return true;
        }

        case RollerStep::Confirm:
        case RollerStep::Done: {
            out = core::Character{};
            out.name = r.name_buf;
            out.race = r.race;
            out.klass = r.klass;
            out.alignment = r.alignment;
            out.attr = r.attr;
            out.char_level = 1;
            out.gold = 100;
            out.armor_class = 10;
            core::recompute_derived(out);
            core::recompute_spell_slots(out);
            out.hp_left = out.hp_max;
            return false;  // signal completion
        }
    }
    return true;
}

}  // namespace wiz::game
