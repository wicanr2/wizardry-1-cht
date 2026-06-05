#include "game/help.h"

#include <SDL.h>

#include <array>
#include <string>
#include <vector>

namespace wiz::game {

namespace {

bool g_active = false;

struct HelpLine { const char* keys; const char* desc; };

const std::vector<HelpLine>* lines_for(Scene s) {
    static const std::vector<HelpLine> title = {
        {"任意鍵", "開始遊戲"},
        {"F1", "顯示此說明"},
    };
    static const std::vector<HelpLine> edge = {
        {"C", "進入城堡"},
        {"T", "訓練場（建立角色）"},
        {"M", "進入迷宮"},
        {"L", "離開遊戲"},
        {"↑↓ Enter", "選單操作"},
        {"F1", "顯示此說明"},
    };
    static const std::vector<HelpLine> castle = {
        {"G", "吉爾伽美什酒館（編隊）"},
        {"A", "冒險者旅館（休息升級）"},
        {"B", "波塔克貿易站（買賣）"},
        {"C", "坎特神殿（治療復活）"},
        {"E", "回到城鎮邊緣"},
        {"F1", "顯示此說明"},
    };
    static const std::vector<HelpLine> maze = {
        {"W / ↑", "前進"},
        {"S / ↓", "後退"},
        {"A / ←", "左轉"},
        {"D / →", "右轉"},
        {"SPACE", "觸發戰鬥"},
        {"C", "進營地（存檔/施法/重排）"},
        {"ESC", "離開迷宮"},
        {"F1", "顯示此說明"},
    };
    static const std::vector<HelpLine> combat = {
        {"F", "攻擊"},
        {"S", "施法（51 個咒語）"},
        {"P", "防禦"},
        {"R", "全隊逃跑（50% 機率）"},
        {"↑↓ ←→ Enter", "選擇法術與目標"},
        {"ESC", "撤退"},
        {"F1", "顯示此說明"},
    };
    static const std::vector<HelpLine> camp = {
        {"S", "存檔"},
        {"I", "檢視角色"},
        {"R", "重排隊伍"},
        {"C", "施法（11 個 utility 咒語）"},
        {"Q", "回標題畫面"},
        {"ESC", "回到迷宮"},
        {"F1", "顯示此說明"},
    };
    static const std::vector<HelpLine> shop = {
        {"Tab", "切換顧客"},
        {"↑↓", "選擇商品"},
        {"B", "購買"},
        {"ESC", "離開商店"},
        {"F1", "顯示此說明"},
    };
    static const std::vector<HelpLine> tavern = {
        {"Tab", "切換名冊/隊伍面板"},
        {"↑↓", "選擇角色"},
        {"A", "加入隊伍"},
        {"R", "移出隊伍"},
        {"ESC", "離開酒館"},
        {"F1", "顯示此說明"},
    };
    static const std::vector<HelpLine> generic = {
        {"↑↓", "選擇"},
        {"Enter", "確認"},
        {"ESC", "返回"},
        {"F1", "顯示此說明"},
    };

    switch (s) {
        case Scene::Title:           return &title;
        case Scene::EdgeOfTown:      return &edge;
        case Scene::Castle:          return &castle;
        case Scene::Maze:            return &maze;
        case Scene::Combat:          return &combat;
        case Scene::Camp:            return &camp;
        case Scene::Shop:            return &shop;
        case Scene::Tavern:          return &tavern;
        case Scene::Inn:
        case Scene::Temple:
        case Scene::TrainingGrounds:
        default:                     return &generic;
    }
}

}  // namespace

bool help_active() { return g_active; }
void toggle_help() { g_active = !g_active; }

void draw_help(Scene scene, const render::UI& ui) {
    if (!g_active) return;

    // Dimmer
    SDL_SetRenderDrawBlendMode(ui.renderer(), SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ui.renderer(), 0, 0, 0, 200);
    SDL_Rect dim{0, 0, 1280, 720};
    SDL_RenderFillRect(ui.renderer(), &dim);

    // Panel
    const int w = 720, h = 480;
    const int x = (1280 - w) / 2, y = (720 - h) / 2;
    ui.draw_frame(x, y, w, h);

    const int pad = 30;
    render::draw_text(ui.renderer(), ui.title_font(),
                      "操作說明 (HELP)",
                      x + w / 2, y + pad, ui.theme().accent,
                      render::Align::Center);

    const auto* lines = lines_for(scene);
    int yy = y + pad + 70;
    for (const auto& L : *lines) {
        // key column (left, accent)
        render::draw_text(ui.renderer(), ui.body_font(), L.keys,
                          x + pad + 80, yy, ui.theme().accent);
        // desc column
        render::draw_text(ui.renderer(), ui.body_font(), L.desc,
                          x + pad + 280, yy, ui.theme().text);
        yy += ui.body_font().line_height() + 8;
    }

    render::draw_text(ui.renderer(), ui.small_font(),
                      "再按 F1 關閉此說明",
                      x + w / 2, y + h - 40, ui.theme().dim,
                      render::Align::Center);
}

}  // namespace wiz::game
