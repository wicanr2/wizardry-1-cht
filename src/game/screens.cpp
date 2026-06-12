#include "game/screens.h"

#include <SDL.h>
#include <sys/stat.h>

#include <cctype>
#include <cstdio>
#include <string>
#include <vector>

#include "core/maze.h"
#include "core/rng.h"
#include "core/rules.h"
#include "data/items_db.h"
#include "game/camp.h"
#include "game/help.h"
#include "game/inn.h"
#include "game/intro.h"
#include "game/traps.h"
#include "game/roller.h"
#include "game/shop.h"
#include "game/tavern.h"
#include "i18n/tr.h"
#include "render/audio.h"
#include "render/auto_map.h"
#include "render/maze_view.h"
#include "render/sprite.h"
#include "render/theme.h"
#include "render/ui.h"
#include "save/gamesave.h"

namespace wiz::game {

namespace {

constexpr int kPadX = 80;
constexpr int kPadY = 110;

struct MenuChoice {
    char hotkey;
    Scene target;
    const char* i18n_key;
};

const std::vector<MenuChoice> kEdgeMenu = {
    {'C', Scene::Castle, "castle"},
    {'T', Scene::TrainingGrounds, "training_grounds"},
    {'M', Scene::Maze, "maze"},
    {'L', Scene::Quit, "leave_game"},
};

const std::vector<MenuChoice> kCastleMenu = {
    {'G', Scene::Tavern, "gilgamesh_s_tavern"},
    {'A', Scene::Inn, "adventurer_s_inn"},
    {'B', Scene::Shop, "boltac_s_trading_post"},
    {'C', Scene::Temple, "cant_s_temple"},
    {'E', Scene::EdgeOfTown, "edge_of_town"},
};

std::vector<render::MenuItem> menu_items_from(const std::vector<MenuChoice>& src) {
    std::vector<render::MenuItem> out;
    out.reserve(src.size());
    for (const auto& c : src) {
        out.push_back({std::string(i18n::tr(c.i18n_key)), c.hotkey, true});
    }
    return out;
}

int hotkey_to_index(const std::vector<MenuChoice>& choices, char k) {
    char up = static_cast<char>(std::toupper(static_cast<unsigned char>(k)));
    for (std::size_t i = 0; i < choices.size(); ++i) {
        if (choices[i].hotkey == up) return static_cast<int>(i);
    }
    return -1;
}

void draw_party_panel(const State& state, const render::UI& ui, int x, int y, int w, int h) {
    ui.draw_frame(x, y, w, h);
    const int pad = 14;
    const auto& body = ui.body_font();
    const auto& small = ui.small_font();

    render::draw_text(ui.renderer(), body, "目前隊伍 / CURRENT PARTY",
                      x + pad, y + pad, ui.theme().accent);

    const int header_y = y + pad + body.line_height() + 8;
    render::draw_text(ui.renderer(), small,
                      "#  名字              職業  AC  HP   狀態",
                      x + pad, header_y, ui.theme().dim);

    int row_y = header_y + small.line_height() + 6;
    const int row_h = body.line_height() + 4;
    if (state.party.count == 0) {
        render::draw_text(ui.renderer(), body, "（無人在隊）",
                          x + pad, row_y, ui.theme().dim);
    } else {
        for (int i = 0; i < state.party.count; ++i) {
            int idx = state.party.roster_index[i];
            if (idx < 0 || static_cast<std::size_t>(idx) >= state.roster.used) continue;
            const auto& c = state.roster.chars[idx];
            // Translate class via i18n catalogue (fighter/mage/priest/... → 戰士/魔法師/牧師/...).
            std::string_view klass_zh = i18n::tr(core::klass_name(c.klass));
            char line[160];
            std::snprintf(line, sizeof(line), "%d  %-16s  %-6.*s  %2d  %d/%d",
                          i + 1,
                          c.name.c_str(),
                          static_cast<int>(klass_zh.size()), klass_zh.data(),
                          int(c.armor_class),
                          int(c.hp_left), int(c.hp_max));
            render::draw_text(ui.renderer(), body, line,
                              x + pad, row_y, ui.theme().text);
            row_y += row_h;
        }
    }
}

void draw_menu_screen(const State& state, const render::UI& ui,
                      std::string_view title_text,
                      const std::vector<MenuChoice>& menu,
                      int hovered) {
    ui.clear();
    ui.draw_title_bar(title_text);

    const int panel_x = kPadX;
    const int panel_y = kPadY;
    const int panel_w = 540;
    const int panel_h = 360;
    draw_party_panel(state, ui, panel_x, panel_y, panel_w, panel_h);

    const int menu_x = panel_x + panel_w + 40;
    const int menu_y = panel_y + 20;
    const int menu_w = 1280 - menu_x - kPadX;
    auto items = menu_items_from(menu);
    ui.draw_menu(menu_x, menu_y, menu_w, items, hovered);

    const int log_x = kPadX;
    const int log_y = panel_y + panel_h + 40;
    const int log_w = 1280 - 2 * kPadX;
    const int log_h = 720 - log_y - 80;
    ui.draw_message_panel(log_x, log_y, log_w, log_h, state.message_log);

    ui.draw_status_bar(state.status_hint);
    ui.present();
}

bool handle_menu(State& state, const SDL_Event* ev, const render::UI& ui,
                 std::string_view title_text, const std::vector<MenuChoice>& menu) {
    static int hovered = 0;
    if (hovered >= static_cast<int>(menu.size())) hovered = 0;

    if (ev && ev->type == SDL_KEYDOWN) {
        SDL_Keycode k = ev->key.keysym.sym;
        bool consumed = false;
        if (k == SDLK_UP) {
            hovered = (hovered - 1 + static_cast<int>(menu.size())) % static_cast<int>(menu.size());
            consumed = true;
        } else if (k == SDLK_DOWN) {
            hovered = (hovered + 1) % static_cast<int>(menu.size());
            consumed = true;
        } else if (k == SDLK_RETURN || k == SDLK_SPACE) {
            auto target = menu[hovered].target;
            if (target == Scene::Quit) return false;
            state.change_scene(target);
            state.push_message(std::string("→ ") +
                               std::string(i18n::tr(menu[hovered].i18n_key)));
            hovered = 0;
            consumed = true;
        } else if (k == SDLK_ESCAPE) {
            if (state.scene == Scene::EdgeOfTown) return false;
            state.change_scene(Scene::EdgeOfTown);
            hovered = 0;
            consumed = true;
        } else {
            char c = (k >= SDLK_a && k <= SDLK_z)
                         ? static_cast<char>(k - SDLK_a + 'A')
                         : static_cast<char>(k);
            int idx = hotkey_to_index(menu, c);
            if (idx >= 0) {
                auto target = menu[idx].target;
                if (target == Scene::Quit) return false;
                state.change_scene(target);
                state.push_message(std::string("→ ") +
                                   std::string(i18n::tr(menu[idx].i18n_key)));
                hovered = 0;
                consumed = true;
            }
        }
        if (consumed) state.dirty = true;
    }

    draw_menu_screen(state, ui, title_text, menu, hovered);
    return true;
}

}  // namespace

namespace {

// Apply per-step poison damage to every Poisoned party member. Returns
// true if any HP change happened (so the caller can refresh the panel).
bool poison_tick_party(State& state) {
    bool any = false;
    for (int i = 0; i < state.party.count; ++i) {
        int ri = state.party.roster_index[i];
        if (ri < 0 || ri >= static_cast<int>(state.roster.chars.size())) continue;
        auto& c = state.roster.chars[ri];
        if (c.status != core::Status::Poisoned || c.poison_strength == 0) continue;
        c.hp_left = static_cast<std::int16_t>(c.hp_left - c.poison_strength);
        any = true;
        if (c.hp_left <= 0) {
            c.status = core::Status::Dead;
            c.poison_strength = 0;
            state.push_message(c.name + " 因中毒身亡。");
        } else {
            char buf[120];
            std::snprintf(buf, sizeof(buf), "%s 因中毒受 %d 傷害(HP %d)。",
                          c.name.c_str(), int(c.poison_strength), int(c.hp_left));
            state.push_message(buf);
        }
    }
    return any;
}

// Record any newly-dead party members as bodies at the current maze
// cell. Idempotent: dead members already recorded are skipped.
void record_dead_bodies(State& state) {
    for (int i = 0; i < state.party.count; ++i) {
        int ri = state.party.roster_index[i];
        if (ri < 0) continue;
        const auto& c = state.roster.chars[ri];
        if (c.status != core::Status::Dead) continue;
        bool already = false;
        for (const auto& b : state.dead_bodies) {
            if (b.roster_idx == ri) { already = true; break; }
        }
        if (already) continue;
        DeadBody b;
        b.roster_idx = ri;
        b.level = state.camera.level;
        b.x = state.camera.x;
        b.y = state.camera.y;
        state.dead_bodies.push_back(b);
        state.push_message(std::string("✝ ") + c.name +
                           " 的屍體留在原地，請日後回來拾取或委託神殿。");
    }
}

// Check if any DeadBody sits on the party's current cell. If so, surface
// a prompt and pick them up (remove from dead_bodies). The character is
// still Status::Dead; take them to Temple for DI / KADORTO.
void check_body_pickup(State& state) {
    auto it = state.dead_bodies.begin();
    while (it != state.dead_bodies.end()) {
        if (it->level == state.camera.level &&
            it->x == state.camera.x && it->y == state.camera.y) {
            const auto& c = state.roster.chars[it->roster_idx];
            state.push_message(std::string("✦ 拾起 ") + c.name +
                               " 的屍體。請帶回神殿復活。");
            it = state.dead_bodies.erase(it);
        } else {
            ++it;
        }
    }
}

// Castle entry auto-cures Poisoned (per Sir-Tech rule: poison stops on
// return to town because the healers' aura cleanses it).
void auto_cure_poison_at_castle(State& state) {
    int cured = 0;
    for (auto& c : state.roster.chars) {
        if (c.status == core::Status::Poisoned) {
            c.status = core::Status::Ok;
            c.poison_strength = 0;
            ++cured;
        }
    }
    if (cured > 0) {
        char buf[80];
        std::snprintf(buf, sizeof(buf), "✦ 城堡治癒了 %d 名中毒者。", cured);
        state.push_message(buf);
    }
}

const char* music_key_for(Scene s) {
    switch (s) {
        case Scene::Title:           return "title";
        case Scene::EdgeOfTown:
        case Scene::Castle:
        case Scene::Tavern:
        case Scene::Shop:
        case Scene::Temple:
        case Scene::Inn:
        case Scene::TrainingGrounds: return "town";
        case Scene::Maze:
        case Scene::Camp:            return "maze";
        case Scene::Combat:          return "combat";
        case Scene::Ending:          return "ending";
        default:                     return nullptr;
    }
}

// Re-evaluated whenever theme changes (so theme cycle restarts music).
int g_music_theme_epoch = 0;
int g_music_last_theme_epoch = -1;

void switch_music_for_scene(Scene s) {
    static Scene last = Scene::Title;
    static bool first = true;
    bool theme_changed = (g_music_theme_epoch != g_music_last_theme_epoch);
    if (!first && last == s && !theme_changed) return;
    first = false;
    last = s;
    g_music_last_theme_epoch = g_music_theme_epoch;
    const char* key = music_key_for(s);
    if (!key) return;
    render::play_music(render::theme::resolve_bgm(key));
}

}  // namespace

static bool scene_tick_dispatch(State& state, const SDL_Event* event,
                                const render::UI& ui);

bool tick(State& state, const SDL_Event* event, const render::UI& ui) {
    switch_music_for_scene(state.scene);

    // F1 toggles help overlay; any key while overlay shown closes it.
    if (event && event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_F1) {
            toggle_help();
            return true;
        }
        // F4 cycles UI language (繁中 → English → 日本語).
        if (event->key.keysym.sym == SDLK_F4) {
            i18n::cycle_lang();
            std::string msg = std::string(i18n::tr("hint_lang_switched_prefix")) +
                              std::string(i18n::lang_display_name(i18n::current_lang()));
            state.push_message(msg);
            return true;
        }
        // F3 cycles visual theme (PCE-CD → Mono → Outline → Sepia → ...).
        // Skips themes whose asset dir is missing.
        if (event->key.keysym.sym == SDLK_F3) {
            render::theme::cycle();
            // Drop the sprite cache so subsequent loads pick up the new
            // theme's PNGs even when keys (resolved paths) overlap.
            render::clear_sprite_cache();
            // Bump the music epoch so the next switch_music_for_scene()
            // call re-evaluates the theme-resolved BGM path.
            ++g_music_theme_epoch;
            std::string msg = std::string(i18n::tr("hint_theme_switched_prefix")) +
                              std::string(render::theme::display_name(
                                              render::theme::current()));
            state.push_message(msg);
            return true;
        }
        if (help_active()) {
            toggle_help();
            return true;
        }
    }

    bool keep = scene_tick_dispatch(state, event, ui);
    if (help_active()) draw_help(state.scene, ui);
    return keep;
}

static bool scene_tick_dispatch(State& state, const SDL_Event* event,
                                const render::UI& ui) {
    switch (state.scene) {
        case Scene::Title:
            // Intro overlay takes over when active.
            if (intro_active()) {
                intro_tick(state, event, ui);
                return true;
            }
            if (event && event->type == SDL_KEYDOWN) {
                SDL_Keycode k = event->key.keysym.sym;
                // 1-5 keys load that save slot directly.
                if (k >= SDLK_1 && k <= SDLK_5) {
                    int slot = k - SDLK_1 + 1;
                    std::string p = save_path_for_slot(slot);
                    if (save::load_game(state, p)) {
                        char buf[200];
                        std::snprintf(buf, sizeof(buf),
                                      "讀取 Slot %d：%s", slot, p.c_str());
                        state.push_message(buf);
                        state.change_scene(Scene::EdgeOfTown);
                    } else {
                        char buf[120];
                        std::snprintf(buf, sizeof(buf),
                                      "Slot %d 沒有存檔。", slot);
                        state.push_message(buf);
                    }
                    return true;
                }
                // First boot (or roster default) shows intro guide.
                // ESC at title goes straight to EdgeOfTown.
                if (k == SDLK_ESCAPE) {
                    state.change_scene(Scene::EdgeOfTown);
                    state.push_message("歡迎來到瘋王的試煉場。");
                } else if (k == SDLK_F2 ||
                           state.roster.used == 0 || true /* always show on first key */) {
                    // Show intro guide
                    start_intro();
                    intro_tick(state, nullptr, ui);
                    return true;
                }
            }
            ui.clear();
            // Theme-aware title background: prefer this theme's splash
            // (assets/themes/<dir>/title/background.png), else fall back to
            // the PCECD splash, else fall through to the text title bar.
            {
                auto try_splash = [&](const std::string& path) -> bool {
                    SDL_Texture* tex = render::load_sprite(ui.renderer(), path);
                    if (!tex) return false;
                    SDL_Rect dst{0, 0, 1280, 720};
                    SDL_RenderCopy(ui.renderer(), tex, nullptr, &dst);
                    return true;
                };
                bool drew = false;
                std::string_view dir = render::theme::dir_name(render::theme::current());
                if (!dir.empty()) {
                    std::string p = std::string(WIZ_ASSETS_DIR) + "/themes/" +
                                    std::string(dir) + "/title/background.png";
                    drew = try_splash(p);
                }
                if (!drew) {
                    drew = try_splash(std::string(WIZ_ASSETS_DIR) +
                                      "/themes/pcecd/title/background.png");
                }
                if (!drew) ui.draw_title_bar(std::string(i18n::tr("ui_game_title")));
            }
            {
                std::vector<std::string> lines = {
                    "Wizardry I: Proving Grounds of the Mad Overlord  v3.2 CHT",
                    "",
                    std::string(i18n::tr("hint_title_slot_keys")),
                    std::string(i18n::tr("hint_title_func_keys")),
                    std::string(i18n::tr("hint_current_theme_prefix")) +
                        std::string(render::theme::display_name(render::theme::current())) +
                        std::string(i18n::tr("hint_current_lang_prefix")) +
                        std::string(i18n::lang_display_name(i18n::current_lang())),
                    "",
                };
                std::string saved_tag(i18n::tr("hint_slot_used"));
                std::string empty_tag(i18n::tr("hint_slot_empty"));
                for (int i = 1; i <= kNumSlots; ++i) {
                    std::string p = save_path_for_slot(i);
                    struct stat st;
                    char line[160];
                    bool exists = (stat(p.c_str(), &st) == 0);
                    std::snprintf(line, sizeof(line), "  Slot %d  %s", i,
                                  exists ? saved_tag.c_str() : empty_tag.c_str());
                    lines.emplace_back(line);
                }
                ui.draw_message_panel(220, 220, 840, 320, lines);
            }
            ui.draw_status_bar("");
            ui.present();
            return true;

        case Scene::EdgeOfTown:
            state.status_hint = std::string(i18n::tr("hint_status_select_enter"));
            return handle_menu(state, event, ui,
                               std::string(i18n::tr("edge_of_town")), kEdgeMenu);

        case Scene::Castle:
            if (state.prev_scene != Scene::Castle) auto_cure_poison_at_castle(state);
            state.status_hint = std::string(i18n::tr("hint_status_castle_back"));
            return handle_menu(state, event, ui,
                               std::string(i18n::tr("castle")), kCastleMenu);

        case Scene::Maze: {
            // Lazy-load all-floor backing store on first entry.
            if (!state.maze_loaded) {
                build_floor(state.mazes[0], 1);
                state.floor_built[0] = true;
                state.maze = state.mazes[0];
                state.camera = {0, 5, 1, render::Facing::North};
                state.maze_loaded = true;
                state.push_message("進入迷宮 B1F。");
                state.push_message("（藏有陷阱：坑/旋轉/傳送/滑梯/樓梯下）");
                render::reveal_from(state.maze, state.camera);
            }

            if (event && event->type == SDL_KEYDOWN) {
                using namespace render;
                SDL_Keycode k = event->key.keysym.sym;
                if (k == SDLK_ESCAPE) {
                    state.change_scene(Scene::EdgeOfTown);
                    state.push_message("逃離迷宮。");
                    return true;
                }
                auto post_step_features = [&]() {
                    // Light tick — MILWA / LOMILWA wear down per step.
                    if (state.light_steps_left > 0) {
                        --state.light_steps_left;
                        if (state.light_steps_left == 0) {
                            state.push_message("** 光芒熄滅，暗影籠罩。");
                        }
                    }
                    // Anti-magic flag persists only while standing on Fizzle.
                    if (feature_at_party(state) != core::SquareFeature::Fizzle) {
                        state.anti_magic_here = false;
                    }
                    // Forced-encounter feature wins over the random roll.
                    if (state.pending_force_encounter) {
                        state.pending_force_encounter = false;
                        render::play(render::Sfx::SwordHit);
                        state.change_scene(Scene::Combat);
                    }
                };
                auto maybe_random_encounter = [&]() {
                    int xx = state.camera.x, yy = state.camera.y;
                    if (state.maze.fights[yy][xx]) {
                        state.push_message("** 強制遭遇！**");
                        render::play(render::Sfx::SwordHit);
                        state.change_scene(Scene::Combat);
                        return;
                    }
                    int p = state.maze.enmy_calc[0];
                    if (p == 0) p = 30 + 7 * (state.camera.level - 1);
                    if (core::global_rng().range(0, 255) < p) {
                        state.push_message("** 遭遇敵人！**");
                        render::play(render::Sfx::SwordHit);
                        state.change_scene(Scene::Combat);
                    }
                };
                if (k == SDLK_UP || k == SDLK_w) {
                    auto facing = state.camera.facing;
                    auto wall = front_wall(state.maze, state.camera.x, state.camera.y, facing);
                    if (wall != core::Wall::Wall) {
                        step_forward(state.camera);
                        render::play(render::Sfx::Footstep);
                        poison_tick_party(state);
                        apply_trap(state, feature_at_party(state));
                        post_step_features();
                        check_body_pickup(state);
                        if (state.scene == Scene::Maze) maybe_random_encounter();
                    } else {
                        state.push_message("** 撞牆 ** WALL!");
                        render::play(render::Sfx::SwordMiss);
                    }
                    if (wall == core::Wall::Door) render::play(render::Sfx::DoorOpen);
                } else if (k == SDLK_DOWN || k == SDLK_s) {
                    step_back(state.camera);
                    render::play(render::Sfx::Footstep);
                    poison_tick_party(state);
                    apply_trap(state, feature_at_party(state));
                    post_step_features();
                    check_body_pickup(state);
                    if (state.scene == Scene::Maze) maybe_random_encounter();
                } else if (k == SDLK_LEFT || k == SDLK_a) {
                    turn_left(state.camera);
                    render::play(render::Sfx::Footstep);
                } else if (k == SDLK_RIGHT || k == SDLK_d) {
                    turn_right(state.camera);
                    render::play(render::Sfx::Footstep);
                } else if (k == SDLK_SPACE) {
                    state.push_message("** 遭遇敵人！**");
                    render::play(render::Sfx::SwordHit);
                    state.change_scene(Scene::Combat);
                } else if (k == SDLK_c) {
                    state.change_scene(Scene::Camp);
                } else if (k == SDLK_m || k == SDLK_F5) {
                    state.show_auto_map = !state.show_auto_map;
                    state.push_message(state.show_auto_map
                                           ? "✦ 自動繪圖：開"
                                           : "✦ 自動繪圖：關");
                } else if (k == SDLK_RETURN) {
                    // Use stairs if standing on them.
                    if (feature_at_party(state) == core::SquareFeature::Stairs) {
                        int next = state.camera.level + 1;
                        if (next > kMaxFloors) {
                            state.push_message("** 已在最深層 (B10F)。**");
                        } else {
                            switch_floor(state, next, 0, 0);
                            char buf[80];
                            std::snprintf(buf, sizeof(buf), "下樓 → B%dF。", next);
                            state.push_message(buf);
                            render::reveal_from(state.maze, state.camera);
                        }
                    }
                }
                render::reveal_from(state.maze, state.camera);
                state.dirty = true;
            }

            ui.clear();
            char title_buf[40];
            std::snprintf(title_buf, sizeof(title_buf), "迷宮 B%dF", state.camera.level);
            ui.draw_title_bar(title_buf);

            // 3D viewport (slightly narrower to make room for auto-map)
            SDL_Rect view_rect{kPadX, kPadY, 540, 480};
            // Dark-zone shading — clamp viewport depth if the party is in
            // a dark cell with no active light spell.
            bool dark_here = false;
            {
                int cx = state.camera.x, cy = state.camera.y;
                if (cx >= 0 && cy >= 0 &&
                    cx < core::MazeLevel::kSize && cy < core::MazeLevel::kSize) {
                    dark_here = state.maze.dark_zone[cy][cx] &&
                                state.light_steps_left == 0;
                }
            }
            render::draw_maze_view(ui.renderer(), state.maze, state.camera,
                                   view_rect, dark_here);

            // Compass + depth overlay, top-right inside the 3D viewport.
            // r expanded so CJK glyphs don't crowd; B?F now lives in its own
            // strip BELOW the rose instead of fighting N/E/S/W for the centre.
            {
                int r = 36;
                int cx = view_rect.x + view_rect.w - r - 14;
                int cy = view_rect.y + r + 14;
                int sl = 14;  // small font line height approx
                SDL_SetRenderDrawColor(ui.renderer(),
                                       ui.theme().panel.r, ui.theme().panel.g,
                                       ui.theme().panel.b, 220);
                SDL_Rect bg{cx - r - 4, cy - r - 4,
                            2*r + 8, 2*r + 8 + sl + 6};
                SDL_RenderFillRect(ui.renderer(), &bg);
                SDL_SetRenderDrawColor(ui.renderer(),
                                       ui.theme().accent.r, ui.theme().accent.g,
                                       ui.theme().accent.b, 255);
                SDL_RenderDrawRect(ui.renderer(), &bg);
                const char* labels[4] = {"N", "E", "S", "W"};
                int facing_idx = static_cast<int>(state.camera.facing);
                int dx[4] = { 0,  r-12,  0,  -r+12};
                int dy[4] = {-r+10,  0,  r-22,  0};
                for (int i = 0; i < 4; ++i) {
                    SDL_Color col = (i == facing_idx) ? ui.theme().accent
                                                     : ui.theme().dim;
                    render::draw_text(ui.renderer(), ui.small_font(), labels[i],
                                      cx + dx[i], cy + dy[i],
                                      col, render::Align::Center);
                }
                // Crosshair in middle so the rose reads even on dark themes.
                SDL_SetRenderDrawColor(ui.renderer(),
                                       ui.theme().dim.r, ui.theme().dim.g,
                                       ui.theme().dim.b, 255);
                SDL_RenderDrawLine(ui.renderer(), cx - 4, cy, cx + 4, cy);
                SDL_RenderDrawLine(ui.renderer(), cx, cy - 4, cx, cy + 4);
                // Depth strip below the rose.
                char dbuf[16];
                std::snprintf(dbuf, sizeof(dbuf), "B%dF", state.camera.level);
                render::draw_text(ui.renderer(), ui.small_font(), dbuf,
                                  cx, cy + r + 2,
                                  ui.theme().text, render::Align::Center);
            }

            // Auto-map ("Eye of Map") — between viewport and info panel.
            // Hidden when state.show_auto_map is false (M key toggles).
            SDL_Rect amap{kPadX + 540 + 16, kPadY, 360, 360};
            if (state.show_auto_map) {
                render::draw_auto_map(ui.renderer(), state.maze, state.camera,
                                      amap, ui.theme());
                render::draw_text(ui.renderer(), ui.small_font(),
                                  "自動繪圖 (Eye of Map) — M 鍵切換",
                                  amap.x + 8, amap.y - 18, ui.theme().dim);
            } else {
                render::draw_text(ui.renderer(), ui.small_font(),
                                  "自動繪圖隱藏中（M 鍵切換）",
                                  amap.x + 8, amap.y - 18, ui.theme().dim);
            }

            // Info panel right side. Height extended so the 3-line hint
            // strip fits inside the frame (was clipped by the status bar
            // before — last hint "ESC 離開迷宮" used to overflow).
            const int info_x = amap.x;
            const int info_y = amap.y + amap.h + 20;
            const int info_w = 1280 - info_x - kPadX;
            const int info_h = 720 - info_y - 70;  // leave 70 px for log + status bar (was 90 — gave bottom hint line no breathing room)
            ui.draw_frame(info_x, info_y, info_w, info_h);
            const char* face_name = "北";
            switch (state.camera.facing) {
                case render::Facing::North: face_name = "北"; break;
                case render::Facing::East:  face_name = "東"; break;
                case render::Facing::South: face_name = "南"; break;
                case render::Facing::West:  face_name = "西"; break;
            }
            char info[128];
            std::snprintf(info, sizeof(info), "位置  X:%d  Y:%d", state.camera.x, state.camera.y);
            render::draw_text(ui.renderer(), ui.body_font(), info,
                              info_x + 14, info_y + 14, ui.theme().text);
            std::snprintf(info, sizeof(info), "面向  %s", face_name);
            render::draw_text(ui.renderer(), ui.body_font(), info,
                              info_x + 14, info_y + 44, ui.theme().accent);
            render::draw_text(ui.renderer(), ui.small_font(),
                              "W/↑前進  S/↓後退",
                              info_x + 14, info_y + 82, ui.theme().dim);
            render::draw_text(ui.renderer(), ui.small_font(),
                              "A/←左轉  D/→右轉",
                              info_x + 14, info_y + 102, ui.theme().dim);
            render::draw_text(ui.renderer(), ui.small_font(),
                              "ESC 離開迷宮  C 營地  M 自動繪圖",
                              info_x + 14, info_y + 122, ui.theme().dim);

            // Bottom message log (sits inside the 90 px reserved above)
            const int log_y = info_y + info_h + 8;
            const int log_h = 720 - log_y - 80;
            ui.draw_message_panel(kPadX, log_y, 1280 - 2 * kPadX, log_h,
                                  state.message_log);

            ui.draw_status_bar(std::string(i18n::tr("hint_status_maze_move")));
            ui.present();
            return true;
        }

        case Scene::Combat: {
            if (state.combat.groups.empty()) {
                // Seed a demo encounter — random monster from data/monsters.json
                core::Monster proto;
                // Pick a monster based on maze depth: lower levels = lower index
                int max_id = std::min<int>(11, static_cast<int>(data::monsters().size()));
                int picked = core::global_rng().range(0, std::max(1, max_id) - 1);
                const auto* entry = data::find_monster_by_id(picked);
                if (entry) {
                    proto.name = entry->name_zh.empty() ? entry->name_en : entry->name_zh;
                    proto.name_unknown = entry->name_unknown.empty()
                                             ? entry->name_en : entry->name_unknown;
                    proto.armor_class = static_cast<std::int8_t>(entry->ac);
                    proto.experience = entry->exp;
                    proto.sprite_path = entry->sprite_path;
                    proto.data_id = entry->id;
                    // Parse hp dice "XdY"
                    auto pos = entry->hp_dice.find('d');
                    if (pos != std::string::npos) {
                        proto.hp_dice_n = static_cast<std::uint8_t>(
                            std::stoi(entry->hp_dice.substr(0, pos)));
                        proto.hp_dice_d = static_cast<std::uint8_t>(
                            std::stoi(entry->hp_dice.substr(pos + 1)));
                    } else {
                        proto.hp_dice_n = 1; proto.hp_dice_d = 8;
                    }
                } else {
                    proto.name = "狗頭人";
                    proto.name_unknown = "狗頭怪物";
                    proto.armor_class = 7;
                    proto.hp_dice_n = 1; proto.hp_dice_d = 4;
                    proto.experience = 415;
                    proto.sprite_path = "sprites/monsters_pcecd/PCECD_MS_Kobold.png";
                }
                core::CombatGroup g;
                g.prototype = proto;
                g.total_count = static_cast<std::int16_t>(core::global_rng().range(1, 4));
                // LATUMAPIC pre-identification — consume the camp-cast flag.
                if (state.latumapic_next_combat) {
                    g.identified = true;
                    state.latumapic_next_combat = false;
                }
                core::begin_combat(state.combat, {g});
                state.combat.active_party_member = 0;
                // Default each character to Parry; user will override.
                core::PlayerAction parry;
                parry.kind = core::PlayerAction::Parry;
                for (int i = 0; i < 6; ++i) core::set_action(state.combat, i, parry);
            }

            // Available spells for the active caster — full W1 catalogue.
            static const char* kSpellList[] = {
                // Mage L1-7
                "HALITO", "MOGREF", "KATINO", "DUMAPIC",
                "DILTO", "SOPIC",
                "MAHALITO", "MOLITO",
                "DALTO", "LAHALITO", "MORLIS",
                "MADALTO", "MAKANITO", "MAMORLIS",
                "HAMAN", "LAKANITO", "ZILWAN", "MASOPIC",
                "MAHAMAN", "TILTOWAIT", "MALOR",
                // Priest L1-7
                "KALKI", "DIOS", "BADIOS", "MILWA", "PORFIC",
                "CALFO", "MANIFO", "MATU", "MONTINO",
                "LOMILWA", "DIALKO", "LATUMAPIC", "BAMATU",
                "DIAL", "BADIAL", "LATUMOFIS", "MAPORFIC",
                "DIALMA", "BADIALMA", "LITOKAN", "KANDI", "DI",
                "LORTO", "MADI", "MABADI", "LOKTOFEIT",
                "MALIKTO", "KADORTO", "MOGATO", "LABADI",
            };
            static int spell_cursor = 0;
            static int target_cursor = 0;
            auto& cb = state.combat;

            auto active_caster = [&]() -> core::Character* {
                int pi = cb.active_party_member;
                if (pi < 0 || pi >= state.party.count) return nullptr;
                int ri = state.party.roster_index[pi];
                if (ri < 0) return nullptr;
                return &state.roster.chars[ri];
            };

            if (event && event->type == SDL_KEYDOWN) {
                SDL_Keycode k = event->key.keysym.sym;
                if (k == SDLK_ESCAPE) {
                    cb.groups.clear();
                    cb.phase = core::CombatPhase::End;
                    cb.outcome = core::CombatOutcome::Fled;
                    state.change_scene(Scene::Maze);
                    return true;
                }
                if (cb.phase == core::CombatPhase::PartyAction) {
                    if (k == SDLK_f) {
                        cb.actions[cb.active_party_member].kind = core::PlayerAction::Fight;
                        cb.actions[cb.active_party_member].target_group = 0;
                        cb.phase = core::CombatPhase::PickTarget;
                        target_cursor = 0;
                        render::play(render::Sfx::MenuPick);
                    } else if (k == SDLK_s) {
                        cb.actions[cb.active_party_member].kind = core::PlayerAction::Spell;
                        cb.phase = core::CombatPhase::PickSpell;
                        spell_cursor = 0;
                        render::play(render::Sfx::MenuPick);
                    } else if (k == SDLK_p) {
                        cb.actions[cb.active_party_member].kind = core::PlayerAction::Parry;
                        ++cb.active_party_member;
                        render::play(render::Sfx::MenuMove);
                    } else if (k == SDLK_r) {
                        for (int i = 0; i < 6; ++i)
                            cb.actions[i].kind = core::PlayerAction::Run;
                        // Trigger resolve next
                        std::array<core::Character, 6> party{};
                        for (int i = 0; i < state.party.count; ++i) {
                            int ri = state.party.roster_index[i];
                            if (ri >= 0) party[i] = state.roster.chars[ri];
                        }
                        int alive_before = 0;
                        for (auto& g : cb.groups) alive_before += g.alive_count;
                        std::array<int, 6> hp_before;
                        for (int i = 0; i < 6; ++i) hp_before[i] = party[i].hp_left;
                        // Pre-cast SFX based on queued action
                        for (int i = 0; i < 6; ++i) {
                            const auto& a = cb.actions[i];
                            if (a.kind != core::PlayerAction::Spell) continue;
                            const std::string& n = a.spell_name;
                            if (n == "DIOS" || n == "DIAL" || n == "DIALMA" || n == "MADI")
                                render::play(render::Sfx::SpellHeal);
                            else if (n.find("HALITO") != std::string::npos ||
                                     n.find("DALTO") != std::string::npos ||
                                     n == "TILTOWAIT" || n == "LITOKAN")
                                render::play(render::Sfx::SpellFire);
                            else
                                render::play(render::Sfx::SpellCast);
                        }

                        core::resolve_round(cb, party);

                        int alive_after = 0;
                        for (auto& g : cb.groups) alive_after += g.alive_count;
                        if (alive_after < alive_before) render::play(render::Sfx::MonsterDie);
                        for (int i = 0; i < 6; ++i) {
                            if (party[i].hp_left < hp_before[i]) {
                                render::play(render::Sfx::PartyDamage);
                                break;
                            }
                        }

                        for (int i = 0; i < state.party.count; ++i) {
                            int ri = state.party.roster_index[i];
                            if (ri >= 0) state.roster.chars[ri] = party[i];
                        }
                        cb.active_party_member = 0;
                    }

                    // Skip over dead/empty members
                    while (cb.active_party_member < state.party.count) {
                        int ri = state.party.roster_index[cb.active_party_member];
                        if (ri < 0 || state.roster.chars[ri].status != core::Status::Ok) {
                            ++cb.active_party_member;
                        } else break;
                    }

                    if (cb.active_party_member >= state.party.count &&
                        cb.outcome == core::CombatOutcome::Ongoing) {
                        // All decisions made — resolve round
                        std::array<core::Character, 6> party{};
                        for (int i = 0; i < state.party.count; ++i) {
                            int ri = state.party.roster_index[i];
                            if (ri >= 0) party[i] = state.roster.chars[ri];
                        }
                        int alive_before = 0;
                        for (auto& g : cb.groups) alive_before += g.alive_count;
                        std::array<int, 6> hp_before;
                        for (int i = 0; i < 6; ++i) hp_before[i] = party[i].hp_left;
                        // Pre-cast SFX based on queued action
                        for (int i = 0; i < 6; ++i) {
                            const auto& a = cb.actions[i];
                            if (a.kind != core::PlayerAction::Spell) continue;
                            const std::string& n = a.spell_name;
                            if (n == "DIOS" || n == "DIAL" || n == "DIALMA" || n == "MADI")
                                render::play(render::Sfx::SpellHeal);
                            else if (n.find("HALITO") != std::string::npos ||
                                     n.find("DALTO") != std::string::npos ||
                                     n == "TILTOWAIT" || n == "LITOKAN")
                                render::play(render::Sfx::SpellFire);
                            else
                                render::play(render::Sfx::SpellCast);
                        }

                        core::resolve_round(cb, party);

                        int alive_after = 0;
                        for (auto& g : cb.groups) alive_after += g.alive_count;
                        if (alive_after < alive_before) render::play(render::Sfx::MonsterDie);
                        for (int i = 0; i < 6; ++i) {
                            if (party[i].hp_left < hp_before[i]) {
                                render::play(render::Sfx::PartyDamage);
                                break;
                            }
                        }

                        for (int i = 0; i < state.party.count; ++i) {
                            int ri = state.party.roster_index[i];
                            if (ri >= 0) state.roster.chars[ri] = party[i];
                        }
                        cb.active_party_member = 0;
                    }
                } else if (cb.phase == core::CombatPhase::PickSpell) {
                    int n = static_cast<int>(std::size(kSpellList));
                    const int cols = 7;
                    if (k == SDLK_UP) spell_cursor = (spell_cursor - cols + n) % n;
                    if (k == SDLK_DOWN) spell_cursor = (spell_cursor + cols) % n;
                    if (k == SDLK_LEFT) spell_cursor = (spell_cursor - 1 + n) % n;
                    if (k == SDLK_RIGHT) spell_cursor = (spell_cursor + 1) % n;
                    if (k == SDLK_RETURN) {
                        cb.actions[cb.active_party_member].spell_name = kSpellList[spell_cursor];
                        cb.phase = core::CombatPhase::PickTarget;
                        target_cursor = 0;
                    }
                } else if (cb.phase == core::CombatPhase::PickTarget) {
                    int n = static_cast<int>(cb.groups.size());
                    if (n > 0) {
                        if (k == SDLK_UP) target_cursor = (target_cursor - 1 + n) % n;
                        if (k == SDLK_DOWN) target_cursor = (target_cursor + 1) % n;
                    }
                    if (k == SDLK_RETURN) {
                        cb.actions[cb.active_party_member].target_group = target_cursor;
                        cb.phase = core::CombatPhase::PartyAction;
                        ++cb.active_party_member;
                        while (cb.active_party_member < state.party.count) {
                            int ri = state.party.roster_index[cb.active_party_member];
                            if (ri < 0 || state.roster.chars[ri].status != core::Status::Ok)
                                ++cb.active_party_member;
                            else break;
                        }
                        if (cb.active_party_member >= state.party.count) {
                            std::array<core::Character, 6> party{};
                            for (int i = 0; i < state.party.count; ++i) {
                                int ri = state.party.roster_index[i];
                                if (ri >= 0) party[i] = state.roster.chars[ri];
                            }
                            core::resolve_round(cb, party);
                            for (int i = 0; i < state.party.count; ++i) {
                                int ri = state.party.roster_index[i];
                                if (ri >= 0) state.roster.chars[ri] = party[i];
                            }
                            cb.active_party_member = 0;
                        }
                    }
                }
                // End-of-combat continue
                if (cb.outcome != core::CombatOutcome::Ongoing &&
                    (k == SDLK_SPACE || k == SDLK_RETURN)) {
                    // Werdna detection — if any group's monster name contains
                    // "WERDNA" and the party survived, this was the final
                    // boss. Route to the ending scene instead of the maze.
                    if (cb.outcome == core::CombatOutcome::Victory) {
                        for (const auto& g : cb.groups) {
                            if (g.prototype.name.find("WERDNA") != std::string::npos) {
                                state.werdna_defeated = true;
                                break;
                            }
                        }
                    }
                    for (const auto& l : cb.log) state.push_message(l);
                    cb.groups.clear();
                    if (state.werdna_defeated) {
                        state.change_scene(Scene::Ending);
                    } else {
                        state.change_scene(Scene::Maze);
                    }
                    return true;
                }
                (void)active_caster;
                state.dirty = true;
            }

            ui.clear();
            ui.draw_title_bar("戰鬥");

            // --- v1.25 LAYOUT — original Wizardry I 1981 style ---
            //
            // Original Apple II combat: the upper-left maze viewport stayed
            // visible and the enemy sprite appeared inside it. The right /
            // bottom region kept party stats + the command menu + the log.
            //
            // We mirror that:
            //   Left  540x480  — maze view as backdrop + monster sprite overlay
            //   Right 524x480  — party status panel
            //   Bottom strip   — log + action prompt

            // 1. Maze 3D backdrop (same dimensions as Scene::Maze viewport).
            SDL_Rect cview{kPadX, kPadY, 540, 480};
            {
                bool dark_combat = false;
                int cx0 = state.camera.x, cy0 = state.camera.y;
                if (cx0 >= 0 && cy0 >= 0 &&
                    cx0 < core::MazeLevel::kSize && cy0 < core::MazeLevel::kSize) {
                    dark_combat = state.maze.dark_zone[cy0][cx0] &&
                                  state.light_steps_left == 0;
                }
                render::draw_maze_view(ui.renderer(), state.maze, state.camera,
                                       cview, dark_combat);
            }

            // 2. Monster sprite(s) overlaid centred in the viewport, sized
            //    so multiple groups still fit side-by-side.
            {
                int n_groups = static_cast<int>(state.combat.groups.size());
                if (n_groups > 0) {
                    int max_total_w = cview.w - 60;
                    int slot_w = std::min(320, max_total_w / n_groups);
                    int slot_h = std::min(320, cview.h - 160);
                    int total_w = slot_w * n_groups + 16 * (n_groups - 1);
                    int start_x = cview.x + (cview.w - total_w) / 2;
                    int sprite_y = cview.y + 50;
                    for (int gi = 0; gi < n_groups; ++gi) {
                        auto& g = state.combat.groups[gi];
                        if (g.prototype.sprite_path.empty()) continue;
                        std::string themed = render::theme::resolve(g.prototype.sprite_path);
                        std::string full = std::string(WIZ_ASSETS_DIR) + "/" + themed;
                        SDL_Texture* tex = render::load_sprite(ui.renderer(), full);
                        if (!tex) continue;
                        int sx = start_x + gi * (slot_w + 16);
                        SDL_Rect dst{sx, sprite_y, slot_w, slot_h};
                        if (g.alive_count == 0) SDL_SetTextureAlphaMod(tex, 60);
                        render::draw_sprite_fit(ui.renderer(), tex, dst);
                        if (g.alive_count == 0) SDL_SetTextureAlphaMod(tex, 255);
                    }
                }
            }

            // 3. Enemy info strip — semi-transparent black band along the
            //    bottom of the viewport, lists every group with AC + HP.
            {
                int strip_h = 22 + 18 * std::max<int>(1, state.combat.groups.size());
                if (strip_h > 120) strip_h = 120;
                int strip_y = cview.y + cview.h - strip_h - 2;
                SDL_SetRenderDrawBlendMode(ui.renderer(), SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(ui.renderer(), 0, 0, 0, 190);
                SDL_Rect strip{cview.x + 4, strip_y, cview.w - 8, strip_h};
                SDL_RenderFillRect(ui.renderer(), &strip);
                SDL_SetRenderDrawColor(ui.renderer(),
                                       ui.theme().accent.r, ui.theme().accent.g,
                                       ui.theme().accent.b, 255);
                SDL_RenderDrawRect(ui.renderer(), &strip);
                int label_y = strip_y + 4;
                for (auto& g : state.combat.groups) {
                    char line[200];
                    std::snprintf(line, sizeof(line), "%d × %s   AC %d   HP約 %d",
                                  int(g.alive_count),
                                  g.identified ? g.prototype.name.c_str()
                                               : g.prototype.name_unknown.c_str(),
                                  int(g.prototype.armor_class),
                                  int(g.hp_total));
                    render::draw_text(ui.renderer(), ui.small_font(), line,
                                      strip.x + 10, label_y,
                                      g.alive_count > 0 ? ui.theme().text : ui.theme().dim);
                    label_y += ui.small_font().line_height() + 2;
                }
            }

            // 4. Party status panel — right column.
            int party_y;
            {
                SDL_Rect party_rect{cview.x + cview.w + 16, kPadY,
                                    1280 - kPadX - (cview.x + cview.w + 16),
                                    cview.h};
                ui.draw_frame(party_rect.x, party_rect.y, party_rect.w, party_rect.h);
                int yy = party_rect.y + 14;
                render::draw_text(ui.renderer(), ui.body_font(), "我方隊伍",
                                  party_rect.x + 14, yy, ui.theme().accent);
                yy += ui.body_font().line_height() + 10;
                for (int i = 0; i < state.party.count; ++i) {
                    int ri = state.party.roster_index[i];
                    if (ri < 0) continue;
                    const auto& c = state.roster.chars[ri];
                    const char* row_tag = (i < 3) ? "前" : "後";
                    char line1[200];
                    std::snprintf(line1, sizeof(line1), "[%s] %d. %s",
                                  row_tag, i + 1, c.name.c_str());
                    char line2[200];
                    std::snprintf(line2, sizeof(line2),
                                  "    HP %d/%d  AC %d  %s",
                                  int(c.hp_left), int(c.hp_max),
                                  int(c.armor_class),
                                  c.status == core::Status::Ok ? "" :
                                  c.status == core::Status::Dead ? "[死亡]" :
                                  c.status == core::Status::Poisoned ? "[中毒]" :
                                  c.status == core::Status::Asleep ? "[沉睡]" :
                                  c.status == core::Status::Paralyzed ? "[麻痺]" :
                                  c.status == core::Status::Stoned ? "[石化]" :
                                  c.status == core::Status::Afraid ? "[恐懼]" : "[?]");
                    SDL_Color col = c.status == core::Status::Ok ? ui.theme().text
                                                                  : ui.theme().dim;
                    render::draw_text(ui.renderer(), ui.small_font(), line1,
                                      party_rect.x + 14, yy, col);
                    render::draw_text(ui.renderer(), ui.small_font(), line2,
                                      party_rect.x + 14, yy + 16, col);
                    yy += 38;
                }
                party_y = party_rect.y + party_rect.h;
            }

            // 5. Log + action prompt — bottom strip.
            int log_y = party_y + 16;
            int log_h = 720 - log_y - 60;
            if (cb.phase == core::CombatPhase::PickSpell) {
                // Draw an empty frame for the spell-picker overlay area
                ui.draw_frame(kPadX, log_y, 1280 - 2 * kPadX, log_h);
            } else {
                ui.draw_message_panel(kPadX, log_y, 1280 - 2 * kPadX, log_h,
                                      std::vector<std::string>(
                                          state.combat.log.end() -
                                              std::min<int>(8, state.combat.log.size()),
                                          state.combat.log.end()));
            }

            // Action prompt panel (replaces or augments log when picking)
            if (cb.outcome == core::CombatOutcome::Ongoing) {
                core::Character* caster = active_caster();
                if (cb.phase == core::CombatPhase::PartyAction && caster) {
                    char prompt_buf[160];
                    std::snprintf(prompt_buf, sizeof(prompt_buf),
                                  "輪到 %s 行動 — [F] 攻擊  [S] 法術  [P] 防禦  [R] 全隊逃跑",
                                  caster->name.c_str());
                    render::draw_text(ui.renderer(), ui.body_font(), prompt_buf,
                                      kPadX + 8, log_y + log_h - 36, ui.theme().accent);
                } else if (cb.phase == core::CombatPhase::PickSpell && caster) {
                    int n = static_cast<int>(std::size(kSpellList));
                    char prompt_buf[120];
                    std::snprintf(prompt_buf, sizeof(prompt_buf),
                                  "%s 選擇法術（↑↓ ←→ 選擇  Enter 確認，%d 個）：",
                                  caster->name.c_str(), n);
                    render::draw_text(ui.renderer(), ui.small_font(), prompt_buf,
                                      kPadX + 8, log_y + 6, ui.theme().accent);
                    // 7 columns × 8 rows = 56 slots, fits 51 with margin
                    const int cols = 7;
                    const int col_w = (1280 - 2 * kPadX) / cols;
                    for (int i = 0; i < n; ++i) {
                        int r = i / cols;
                        int co = i % cols;
                        SDL_Color col = (i == spell_cursor)
                                            ? SDL_Color{255, 255, 0, 255}
                                            : ui.theme().text;
                        char b2[64];
                        std::snprintf(b2, sizeof(b2), "%s%s",
                                      i == spell_cursor ? "▸" : "  ",
                                      kSpellList[i]);
                        render::draw_text(ui.renderer(), ui.small_font(), b2,
                                          kPadX + 14 + co * col_w,
                                          log_y + 26 + r * 16,
                                          col);
                    }
                } else if (cb.phase == core::CombatPhase::PickTarget && caster) {
                    char prompt_buf[120];
                    std::snprintf(prompt_buf, sizeof(prompt_buf),
                                  "%s 選擇目標（↑↓ 選擇  Enter 確認）：",
                                  caster->name.c_str());
                    render::draw_text(ui.renderer(), ui.body_font(), prompt_buf,
                                      kPadX + 8, log_y + 10, ui.theme().accent);
                    for (std::size_t i = 0; i < cb.groups.size(); ++i) {
                        SDL_Color col = (static_cast<int>(i) == target_cursor)
                                            ? SDL_Color{255, 255, 0, 255}
                                            : ui.theme().text;
                        char b2[120];
                        std::snprintf(b2, sizeof(b2), "%s 第 %zu 群 (%d 隻)",
                                      static_cast<int>(i) == target_cursor ? "→" : "  ",
                                      i + 1,
                                      int(cb.groups[i].alive_count));
                        render::draw_text(ui.renderer(), ui.body_font(), b2,
                                          kPadX + 24, log_y + 40 + static_cast<int>(i) * 28,
                                          col);
                    }
                }
            }

            const char* hint;
            switch (cb.phase) {
                case core::CombatPhase::PickSpell:
                    hint = "↑↓ 選法術  Enter 確認  ESC 撤退";
                    break;
                case core::CombatPhase::PickTarget:
                    hint = "↑↓ 選目標  Enter 確認  ESC 撤退";
                    break;
                default:
                    hint = cb.outcome == core::CombatOutcome::Ongoing
                               ? "F 攻擊  S 法術  P 防禦  R 全隊逃跑  ESC 撤退"
                               : "戰鬥結束 - 按 SPACE/Enter 繼續";
                    break;
            }
            ui.draw_status_bar(hint);
            ui.present();
            return true;
        }

        case Scene::TrainingGrounds: {
            // Lazy-init the roller
            if (!state.roller) {
                state.roller = std::make_shared<RollerState>();
                SDL_StartTextInput();
                // Drop any text-input events that fired as part of the
                // hotkey ('T') that brought us here.
                SDL_FlushEvent(SDL_TEXTINPUT);
                return true;
            }
            auto* r = static_cast<RollerState*>(state.roller.get());
            core::Character new_char;
            bool active = roller_tick(*r, state, event, ui, new_char);
            if (!active && r->step == RollerStep::Done) {
                // Save into roster
                if (state.roster.used < save::Roster::kMaxChars) {
                    state.roster.chars[state.roster.used++] = new_char;
                    state.push_message(std::string("✦ 新角色：") + new_char.name +
                                       "  Lv1  HP " + std::to_string(new_char.hp_max));
                }
                SDL_StopTextInput();
                state.roller.reset();
                state.change_scene(Scene::EdgeOfTown);
            } else if (!active) {
                SDL_StopTextInput();
                state.roller.reset();
            }
            return true;
        }

        case Scene::Shop:
            return shop_tick(state, event, ui);
        case Scene::Inn:
            return inn_tick(state, event, ui);
        case Scene::Temple:
            return temple_tick(state, event, ui);
        case Scene::Tavern:
            return tavern_tick(state, event, ui);
        case Scene::Camp:
            return camp_tick(state, event, ui);

        case Scene::Ending: {
            // Werdna victory epilogue — splash + epilogue text, any key
            // returns to the title screen. Theme-aware: prefers the PCE-CD
            // composited ending art if the file exists.
            if (event && event->type == SDL_KEYDOWN) {
                state.werdna_defeated = false;
                state.maze_loaded = false;
                state.change_scene(Scene::Title);
                return true;
            }
            ui.clear();
            {
                std::string_view dir =
                    render::theme::dir_name(render::theme::current());
                bool drew = false;
                auto try_splash = [&](const std::string& path) -> bool {
                    SDL_Texture* tex = render::load_sprite(ui.renderer(), path);
                    if (!tex) return false;
                    SDL_Rect dst{0, 0, 1280, 720};
                    SDL_RenderCopy(ui.renderer(), tex, nullptr, &dst);
                    return true;
                };
                if (!dir.empty()) {
                    drew = try_splash(std::string(WIZ_ASSETS_DIR) +
                                      "/themes/" + std::string(dir) +
                                      "/ending/background.png");
                }
                if (!drew) {
                    drew = try_splash(std::string(WIZ_ASSETS_DIR) +
                                      "/themes/pcecd/ending/background.png");
                }
                if (!drew) {
                    SDL_SetRenderDrawColor(ui.renderer(), 8, 6, 18, 255);
                    SDL_Rect full{0, 0, 1280, 720};
                    SDL_RenderFillRect(ui.renderer(), &full);
                }
            }
            // Banner + epilogue text panel — i18n-routed.
            render::draw_text(ui.renderer(), ui.title_font(),
                              std::string(i18n::tr("ending_banner")),
                              640, 90, ui.theme().accent,
                              render::Align::Center);
            {
                std::vector<std::string> lines = {
                    std::string(i18n::tr("ending_epilogue_l1")),
                    std::string(i18n::tr("ending_epilogue_l2")),
                    "",
                    std::string(i18n::tr("ending_epilogue_l3")),
                    std::string(i18n::tr("ending_epilogue_l4")),
                    "",
                    std::string(i18n::tr("ending_press_to_continue")),
                };
                ui.draw_message_panel(120, 470, 1040, 220, lines);
            }
            ui.present();
            return true;
        }

        case Scene::Quit:
            return false;
    }
    return true;
}

}  // namespace wiz::game
