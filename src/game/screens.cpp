#include "game/screens.h"

#include <SDL.h>

#include <cctype>
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
#include "game/roller.h"
#include "game/shop.h"
#include "game/tavern.h"
#include "i18n/tr.h"
#include "render/audio.h"
#include "render/auto_map.h"
#include "render/maze_view.h"
#include "render/sprite.h"
#include "render/ui.h"

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

const char* music_for(Scene s) {
    switch (s) {
        case Scene::Title:           return "audio/title.mp3";
        case Scene::EdgeOfTown:
        case Scene::Castle:
        case Scene::Tavern:
        case Scene::Shop:
        case Scene::Temple:
        case Scene::Inn:
        case Scene::TrainingGrounds: return "audio/town.mp3";
        case Scene::Maze:
        case Scene::Camp:            return "audio/maze.mp3";
        case Scene::Combat:          return "audio/combat.mp3";
        default:                     return nullptr;
    }
}

void switch_music_for_scene(Scene s) {
    static Scene last = Scene::Title;
    static bool first = true;
    if (!first && last == s) return;
    first = false;
    last = s;
    const char* track = music_for(s);
    if (!track) return;
    std::string full = std::string(WIZ_ASSETS_DIR) + "/" + track;
    render::play_music(full);
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
                // First boot (or roster default) shows intro guide.
                // ESC at title goes straight to EdgeOfTown.
                if (event->key.keysym.sym == SDLK_ESCAPE) {
                    state.change_scene(Scene::EdgeOfTown);
                    state.push_message("歡迎來到瘋王的試煉場。");
                } else if (event->key.keysym.sym == SDLK_F2 ||
                           state.roster.used == 0 || true /* always show on first key */) {
                    // Show intro guide
                    start_intro();
                    intro_tick(state, nullptr, ui);
                    return true;
                }
            }
            ui.clear();
            ui.draw_title_bar("巫術：瘋王的試煉場");
            ui.draw_message_panel(260, 260, 760, 200,
                                  {"Wizardry I: Proving Grounds of the Mad Overlord  v3.2 CHT",
                                   "",
                                   "任意鍵：開始新手導覽（按 F1 / F2 隨時叫出說明）",
                                   "ESC：直接進城鎮邊緣（跳過導覽）"});
            ui.draw_status_bar("");
            ui.present();
            return true;

        case Scene::EdgeOfTown:
            state.status_hint = "↑↓ 選擇   Enter 確認   字母鍵直接選   ESC 離開";
            return handle_menu(state, event, ui,
                               std::string(i18n::tr("edge_of_town")), kEdgeMenu);

        case Scene::Castle:
            state.status_hint = "↑↓ 選擇   Enter 確認   ESC 回到城鎮邊緣";
            return handle_menu(state, event, ui,
                               std::string(i18n::tr("castle")), kCastleMenu);

        case Scene::Maze: {
            // Lazy-load a demo maze on first entry
            if (!state.maze_loaded) {
                using core::Wall;
                // Build a small playable corridor in the top-left corner
                auto& m = state.maze;
                m.level_number = 1;
                for (int y = 0; y < core::MazeLevel::kSize; ++y) {
                    for (int x = 0; x < core::MazeLevel::kSize; ++x) {
                        m.west[y][x]  = (x == 0) ? Wall::Wall : Wall::Open;
                        m.east[y][x]  = (x == core::MazeLevel::kSize - 1) ? Wall::Wall : Wall::Open;
                        m.north[y][x] = (y == 0) ? Wall::Wall : Wall::Open;
                        m.south[y][x] = (y == core::MazeLevel::kSize - 1) ? Wall::Wall : Wall::Open;
                    }
                }
                // Carve a few internal walls to make the view interesting
                m.east[0][0] = Wall::Wall; m.west[0][1] = Wall::Wall;
                m.east[1][2] = Wall::Door; m.west[1][3] = Wall::Door;
                m.south[2][3] = Wall::Wall; m.north[3][3] = Wall::Wall;
                m.east[3][5] = Wall::Wall; m.west[3][6] = Wall::Wall;
                state.camera = {0, 5, 1, render::Facing::North};
                state.maze_loaded = true;
                state.push_message("進入迷宮 B1F。");
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
                if (k == SDLK_UP || k == SDLK_w) {
                    auto facing = state.camera.facing;
                    auto wall = front_wall(state.maze, state.camera.x, state.camera.y, facing);
                    if (wall != core::Wall::Wall) {
                        step_forward(state.camera);
                        render::play(render::Sfx::Footstep);
                    } else {
                        state.push_message("** 撞牆 ** WALL!");
                        render::play(render::Sfx::SwordMiss);
                    }
                    if (wall == core::Wall::Door) render::play(render::Sfx::DoorOpen);
                } else if (k == SDLK_DOWN || k == SDLK_s) {
                    step_back(state.camera);
                    render::play(render::Sfx::Footstep);
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
                }
                render::reveal_from(state.maze, state.camera);
                state.dirty = true;
            }

            ui.clear();
            ui.draw_title_bar("迷宮 B1F");

            // 3D viewport (slightly narrower to make room for auto-map)
            SDL_Rect view_rect{kPadX, kPadY, 540, 480};
            render::draw_maze_view(ui.renderer(), state.maze, state.camera,
                                   view_rect, ui.theme());

            // Auto-map ("Eye of Map") — between viewport and info panel
            SDL_Rect amap{kPadX + 540 + 16, kPadY, 360, 360};
            render::draw_auto_map(ui.renderer(), state.maze, state.camera,
                                  amap, ui.theme());
            render::draw_text(ui.renderer(), ui.small_font(),
                              "自動繪圖 (Eye of Map)",
                              amap.x + 8, amap.y - 18, ui.theme().dim);

            // Info panel right side
            const int info_x = amap.x;
            const int info_y = amap.y + amap.h + 20;
            const int info_w = 1280 - info_x - kPadX;
            const int info_h = kPadY + 480 - info_y;
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
                              info_x + 14, info_y + 14 + 30, ui.theme().accent);
            render::draw_text(ui.renderer(), ui.small_font(),
                              "W/↑前進  S/↓後退",
                              info_x + 14, info_y + 80, ui.theme().dim);
            render::draw_text(ui.renderer(), ui.small_font(),
                              "A/←左轉  D/→右轉",
                              info_x + 14, info_y + 100, ui.theme().dim);
            render::draw_text(ui.renderer(), ui.small_font(),
                              "ESC 離開迷宮",
                              info_x + 14, info_y + 120, ui.theme().dim);

            // Bottom message log
            const int log_y = info_y + info_h + 30;
            const int log_h = 720 - log_y - 80;
            ui.draw_message_panel(kPadX, log_y, 1280 - 2 * kPadX, log_h,
                                  state.message_log);

            ui.draw_status_bar("WASD / 方向鍵 移動   ESC 返回");
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
                    for (const auto& l : cb.log) state.push_message(l);
                    cb.groups.clear();
                    state.change_scene(Scene::Maze);
                    return true;
                }
                (void)active_caster;
                state.dirty = true;
            }

            ui.clear();
            ui.draw_title_bar("戰鬥");

            // Enemy panel — top (with sprite column)
            const int enemy_panel_h = 140;
            ui.draw_frame(kPadX, kPadY, 1280 - 2 * kPadX, enemy_panel_h);
            const int sprite_w = 90;
            int xx = kPadX + 14;
            int yy = kPadY + 14;
            render::draw_text(ui.renderer(), ui.body_font(), "敵方", xx, yy, ui.theme().accent);
            yy += ui.body_font().line_height() + 4;
            for (auto& g : state.combat.groups) {
                // Sprite (left)
                if (!g.prototype.sprite_path.empty()) {
                    std::string full = std::string(WIZ_ASSETS_DIR) + "/" + g.prototype.sprite_path;
                    SDL_Texture* tex = render::load_sprite(ui.renderer(), full);
                    if (tex) {
                        SDL_Rect dst{xx, yy - 4, sprite_w, enemy_panel_h - 60};
                        render::draw_sprite_fit(ui.renderer(), tex, dst);
                    }
                }
                char line[200];
                std::snprintf(line, sizeof(line), "  %d × %s   (AC %d, HP約 %d)",
                              int(g.alive_count),
                              g.identified ? g.prototype.name.c_str() :
                                             g.prototype.name_unknown.c_str(),
                              int(g.prototype.armor_class),
                              int(g.hp_total));
                render::draw_text(ui.renderer(), ui.body_font(), line,
                                  xx + sprite_w + 16, yy,
                                  g.alive_count > 0 ? ui.theme().text : ui.theme().dim);
                yy += ui.body_font().line_height() + 2;
            }

            // Party panel - middle
            int party_y = kPadY + 160;
            ui.draw_frame(kPadX, party_y, 1280 - 2 * kPadX, 220);
            yy = party_y + 14;
            render::draw_text(ui.renderer(), ui.body_font(), "我方隊伍",
                              kPadX + 14, yy, ui.theme().accent);
            yy += ui.body_font().line_height() + 4;
            for (int i = 0; i < state.party.count; ++i) {
                int ri = state.party.roster_index[i];
                if (ri < 0) continue;
                const auto& c = state.roster.chars[ri];
                char line[200];
                std::snprintf(line, sizeof(line), "  %d. %-12s  HP %d/%d  AC %d  %s",
                              i + 1, c.name.c_str(),
                              int(c.hp_left), int(c.hp_max),
                              int(c.armor_class),
                              c.status == core::Status::Dead ? "[死亡]" : "");
                render::draw_text(ui.renderer(), ui.small_font(), line,
                                  kPadX + 14, yy,
                                  c.status == core::Status::Ok
                                      ? ui.theme().text
                                      : ui.theme().dim);
                yy += ui.small_font().line_height() + 4;
            }

            // Log panel - bottom; hide log when picking spell to make room.
            int log_y = party_y + 240;
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

        case Scene::Quit:
            return false;
    }
    return true;
}

}  // namespace wiz::game
