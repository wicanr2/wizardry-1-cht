#include "game/intro.h"

#include <SDL.h>

#include <array>
#include <string>

namespace wiz::game {

namespace {

bool g_active = false;
int g_step = 0;

struct Slide {
    const char* title;
    const char* body[8];
};

constexpr std::array<Slide, 5> kSlides = {{
    {"歡迎來到瘋王試煉場", {
        "你是新加入的冒險者隊伍。",
        "瘋王特雷波（Trebor）的城堡下方藏著一座 10 層迷宮。",
        "邪惡法師沃登納（Werdna）偷走了 Trebor 的護身符，",
        "正藏在 B10F 等你來找他算帳。",
        "",
        "（按任意鍵繼續，ESC 跳過）",
    }},
    {"第一步：到城堡集合", {
        "進入「城鎮邊緣」後，先按 C 進城堡。",
        "城堡 5 個地點：",
        "  G 酒館：編組你的小隊（最多 6 人）",
        "  A 旅館：休息恢復 HP / 升級",
        "  B 商店：買武器與盔甲",
        "  C 神殿：治療負面狀態、復活死者",
        "  E 退回城鎮邊緣",
    }},
    {"第二步：訓練場開角色", {
        "城鎮邊緣按 T 進訓練場。",
        "8 步開角：名字 → 種族（人類 / 精靈 / 矮人 / 侏儒 / 霍比特）",
        "        → 陣營（善良 / 中立 / 邪惡）",
        "        → 屬性 → 分配紅利點 → 職業（4 基職 + 4 進階）",
        "",
        "新手建議：2 戰士 + 1 牧師 + 1 盜賊 + 2 魔法師。",
    }},
    {"第三步：下迷宮", {
        "城鎮邊緣按 M 進迷宮。",
        "WASD 或方向鍵移動；空白鍵觸發戰鬥。",
        "畫面右側「自動繪圖」會記錄走過的地方。",
        "按 C 進營地，可以存檔、檢視角色、施法治療。",
        "",
        "永遠記得回城堡 — 死了沒復活就再見了。",
    }},
    {"第四步：戰鬥與咒語", {
        "戰鬥按鍵：F 攻擊 / S 法術 / P 防禦 / R 全隊逃跑。",
        "法術 51 個：HALITO（小火）→ MAHALITO（火球）→ TILTOWAIT（滅世）",
        "施法選法術後再選目標（敵人群）。",
        "",
        "**任何時候按 F1 都會顯示當前場景的按鍵說明**。",
        "祝你好運！按任意鍵開始冒險。",
    }},
}};

}  // namespace

bool intro_active() { return g_active; }
void start_intro() { g_active = true; g_step = 0; }

bool intro_tick(State& state, const SDL_Event* event, const render::UI& ui) {
    if (!g_active) return false;

    if (event && event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_ESCAPE) {
            g_active = false;
            state.change_scene(Scene::EdgeOfTown);
            return false;
        }
        ++g_step;
        if (g_step >= static_cast<int>(kSlides.size())) {
            g_active = false;
            state.change_scene(Scene::EdgeOfTown);
            return false;
        }
    }

    // Render
    ui.clear();
    SDL_SetRenderDrawColor(ui.renderer(), 0, 0, 0, 255);
    SDL_Rect bg{0, 0, 1280, 720};
    SDL_RenderFillRect(ui.renderer(), &bg);

    // Decorative top/bottom bars
    SDL_SetRenderDrawColor(ui.renderer(),
                           ui.theme().border.r, ui.theme().border.g,
                           ui.theme().border.b, 255);
    SDL_Rect top{40, 30, 1280 - 80, 2};
    SDL_RenderFillRect(ui.renderer(), &top);
    SDL_Rect bot{40, 720 - 60, 1280 - 80, 2};
    SDL_RenderFillRect(ui.renderer(), &bot);

    const auto& slide = kSlides[g_step];

    // Title
    render::draw_text(ui.renderer(), ui.title_font(), slide.title,
                      1280 / 2, 70, ui.theme().accent, render::Align::Center);

    // Body lines
    int yy = 200;
    int line_h = ui.body_font().line_height() + 14;
    for (const char* l : slide.body) {
        if (!l) break;
        render::draw_text(ui.renderer(), ui.body_font(), l,
                          1280 / 2, yy, ui.theme().text, render::Align::Center);
        yy += line_h;
    }

    // Progress dots
    int dots_x = 1280 / 2 - (kSlides.size() * 16) / 2;
    int dots_y = 720 - 100;
    for (std::size_t i = 0; i < kSlides.size(); ++i) {
        SDL_Color c = (static_cast<int>(i) == g_step)
                          ? ui.theme().accent : ui.theme().dim;
        SDL_SetRenderDrawColor(ui.renderer(), c.r, c.g, c.b, 255);
        SDL_Rect dot{dots_x + static_cast<int>(i) * 16, dots_y, 8, 8};
        SDL_RenderFillRect(ui.renderer(), &dot);
    }

    render::draw_text(ui.renderer(), ui.small_font(),
                      "任意鍵 → 下一頁    ESC → 跳過所有教學",
                      1280 / 2, 720 - 30, ui.theme().dim, render::Align::Center);

    ui.present();
    return true;
}

}  // namespace wiz::game
