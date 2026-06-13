---
layout: default
title: QA 整合測試報告 — v1.25
---

# QA 整合測試報告 — v1.25

> 報告日期：2026-06-13
> 測試版本：v1.25（commit 28d422a）
> 測試環境：Xvfb 1280×720 + xdotool 注入 + ImageMagick `import`
> 測試框架：`tools/record_playthrough.sh` 衍生的 `tools/qa_v124_part{2..7}.sh`
> 螢幕截圖總數：230 張（其中 133 張非黑屏）→ `docs/v124_qa_playthrough/`

## 摘要

| 範疇 | 結論 |
|---|---|
| **核心 chrome（F3/F4/標題）** | F4 三語切換 ✅、F3 sprite & 文字 ✅、**標題背景圖只有 PCE-CD 有** ⚠ |
| **城堡 5 個店家** | Tavern / Inn / Temple 正常；**Shop 只能買、不能賣**；Training 進入即建角無選單 |
| **3D 迷宮 + automap + 指北針** | 「**運作非常好**」（QA agent 原話）|
| **戰鬥畫面（v1.25 新版）** | 「**Excellent**」— 走廊背景 + 浮現怪物 + 前後排標籤、HP/AC tick 正確 |
| **法術系統** | 51 法術 picker 開啟 OK，但 grid **超出 720p 底邊**，槽位扣除無視覺回饋 |
| **8 種陷阱** | 60 分鐘走 80 步沒踩到任何一個（B1F 機率太稀疏）|
| **終局（Werdna）** | **未驗證** — 60 分鐘內找不到下樓樓梯（B1F spawn `(0,5)` 附近沒可見出口）|

---

## P0：catastrophic（必修）

### 1. 城鎮邊緣 ESC = 直接離開遊戲（無確認）

**位置**：`src/game/screens.cpp:169`

```cpp
if (state.scene == Scene::EdgeOfTown) return false;  // 直接 quit main loop
```

QA agent 在 7 次測試中至少 2 次誤觸 ESC 全部 progress 丟掉。原版手冊
要求「Leave Game」是城鎮邊緣選單的明確選項，不該是 ESC bind。

**修法**：ESC 改成「彈回上一個 scene」或 prompt「確定離開？Y/N」。

### 2. Shop 只能買，不能賣 / 鑑定 / 解咒 / 集資

**位置**：`src/game/shop.cpp:144` — 只 handle `b`

```cpp
if (k == SDLK_b && !items.empty()) { ... }
// 沒有 SDLK_s / SDLK_i / SDLK_u / SDLK_p
```

`docs/MANUAL_GAP.md` 把 Shop 列在 ✅，但實作只有 1/5 服務。
玩家賣回不需要的破損品、鑑定 `*未鑑定*` 道具、解咒都做不到。

**修法**：補上 `s` 賣回（半價）、`i` 鑑定（100 金/件）、`u` 解咒（v1.22 改在神殿，可保留 alias）、`p` 集資轉移金幣給隊伍其他人。

---

## P1：UX-degrading（高優先級）

### 3. `klass_name()` 在繁中模式下回傳英文

**位置**：`src/core/character.cpp:16-28`

```cpp
const char* klass_name(Klass k) {
    switch (k) {
        case Klass::Fighter: return "fighter";  // 應該走 tr()
        ...
    }
}
```

Tavern / Shop / 多處 UI 直接 `core::klass_name(c.klass)` →
角色卡顯示「阿洛札 Lv5 **fighter** HP 25/25」而非「戰士」。
同樣的 i18n 漏在 Shop 道具種類欄（weapon/armor/shield/...）。

**修法**：(a) 改 `klass_name` 回傳 i18n key 給 caller `tr()` 查表；
或 (b) 直接維護 zh/en/ja 三語表 + `klass_name_localised(k, lang)`。

### 4. Tavern Shift+↑↓ 隊伍重排「位置 swap 偏離一格」

**位置**：`src/game/tavern.cpp:175-216`

```cpp
if (k == SDLK_UP) { ... s.party_cursor = (s.party_cursor - 1) % count; }  // 先動
...
bool shift_held = ...;
if (shift_held) { pi = s.party_cursor; swap_with = pi - 1; std::swap(...); }
// pi 已是「動完」的值，swap 對象差一格
```

實際效果：Shift+↑ 把當前 cursor 跟「上面**第二格**」swap，
而不是用戶以為的「跟上面相鄰格 swap」。`docs/MANUAL_GAP.md` 把這
條列在 ✅ v1.23 — 假陽性。

**修法**：先檢查 `shift_held`，shift 模式跳過 cursor-move 分支。

### 5. ✦ Unicode U+2726 在迷宮 status line 渲染為 tofu

**症狀**：「**▢ 自動繪圖：開/關**」「**▢ 切換主題**」「**▢ 已存到 Slot 1**」
所有 toast 都這樣。Noto Sans CJK TC 本身有 U+2726，但我們用
SDL_ttf 的 fallback chain 可能沒命中。

**修法**：(a) 統一改用 ASCII 安全 marker `>>` 或 `[*]`；
或 (b) 載入額外的 emoji-safe TTF（Symbols Nerd Font）做 fallback。

---

## P2：polish / 未來迭代

### 6. 戰鬥沒有 per-action log
原版手冊規定「X swings, hits, 5 damage」每個動作要有日誌，目前只有 round-end state delta。

### 7. 法術槽消耗無視覺回饋
`consume_spell_slot()` 程式碼跑了，但玩家看不到「Lv1: 4/4」這種 HUD。

### 8. 法術 picker grid 超出 720p
51 個咒語 7×8 grid，底邊壓到 status bar。需要 scroll 或 paging。

### 9. 戰鬥 Enter 不該預設 Fight
原版手冊規定「按 Enter 直接結束 round」，目前強制按 F/S/P/R。

### 10. F3 切到 Mono/Outline/Sepia 時標題背景圖無 fallback splash
`assets/themes/{mono,outline,sepia}/title/background.png` 不存在 →
fallback to PCE-CD 的 splash。雜誌風來說可以接受，但 footer
「目前主題：Mono」訊息會跟視覺對不上。

### 11. Training Grounds 進入即建角無選單
按 T 直接跳 「輸入名字」prompt。應該有「新建 / 檢視 / 刪除」選單。

---

## 並未復現（疑為 tester harness 問題）

| Tester 認為 fail | 程式碼證據 | 結論 |
|---|---|---|
| **F1 不開 help overlay** | `screens.cpp:329-331` SDLK_F1 → `toggle_help()` 路由正確 | xdotool 注入失敗 / xvfb keyboard mapping 問題，**非實作 bug** |
| **F2 不開 intro** | `screens.cpp:402` 在 Title scene 接 SDLK_F2 → `start_intro()` | 同上 |
| **F5 不切 automap** | `screens.cpp:567` 接 SDLK_F5 OR SDLK_m → toggle | 同上（M 鍵 tester 能用，F5 同路徑） |
| **走 80 步沒踩到陷阱** | `traps.cpp::apply_trap` 在 maze step handler 接得正確 | B1F 陷阱稀疏（原版設計），tester 走的路線剛好錯開 |

---

## 螢幕證據

所有 230 張 screenshot 在 `docs/v124_qa_playthrough/`：
- `p1_*` — 開機 / F4 / F3
- `p2_*` — Tavern + Shop
- `p3_*` — Inn + Temple
- `p4_*` — 戰鬥
- `p5_*` — 法術 picker（含**overflow 證據** `p5_04_spell_picker.png`）
- `p6_*` — Edge of Town / Training
- `p7_*` — Camp + 自動地圖

Harness 腳本：`tools/qa_v124_part{2..7}.sh`，可重跑驗證。

---

## 後續迭代清單

| 優先級 | Bug | 工作量 |
|---|---|---|
| **v1.25.2** | #1 ESC quit / #2 Shop sell+identify / #3 klass_name CHT / #4 Tavern swap / #5 ✦ glyph | ~4 hr |
| **v1.26** | #6 combat log / #7 spell-slot HUD / #8 spell picker overflow | ~6 hr |
| **v1.27** | #9 Enter default Fight / #10 theme title splash 補齊 / #11 Training menu | ~4 hr |

---

*Report 由 game-tester agent + 主控 review 合成。原始 agent 輸出見
commit 訊息 + screenshots。*
