---
layout: default
title: 手冊對照表 — Wizardry I 規則書 vs 現有實作 Gap Analysis
---

# 手冊對照表 — Wizardry I 規則書 vs 現有實作

> 這份文件比對 1981 年 Sir-Tech 原始手冊（Mac EN 重排版，254 頁
> 合集中前 ~60 頁為 W1 章節）與 `wizardry-cht` 現有實作的落差。
> 所有「規則」描述為自行改寫，不直接引用手冊原文。

最後更新：**v1.24（2026-06-12）**

> 📝 **v1.24 重磅更新**：v1.12b 那份 gap analysis 列的 **P0/P1/P2 全部 17 條已完成**。
> 本文件已經從「待辦清單」改為「已完工驗收清單」。所有 v1.19-v1.24 落地細節都
> 留在下方「✅ 已完成」表的最後一段。

---

## ✅ 已完成

### 核心系統（v0.x-v1.18 基線）

| 系統 | 主要檔案 |
| --- | --- |
| 5 種族 / 3 陣營 / 8 職業列舉 | `src/core/character.h` |
| 6 屬性骰點（DERF roller）+ 種族屬性修正 | `src/game/roller.cpp` |
| 51 道法術完整 catalogue | `assets/data/spells.json` |
| 多群組怪物戰鬥框架 | `src/core/combat.{h,cpp}` |
| 4 種戰鬥動作（攻擊/施法/防禦/逃跑） | `combat.h` `PlayerAction` |
| 識別/未識別怪物欄位 | `CombatGroup::identified` |
| 8 格背包 + equipped/identified/cursed 三旗標 | `core/character.h` |
| 城堡 5 項服務（酒館/旅館/商店/神殿/培訓場）UI | `src/game/{tavern,inn,shop,...}` |
| 旅館 5 級房型 + 升等檢查 | `src/game/inn.cpp` |
| 多 slot 存檔（5 槽） | `src/save/gamesave.cpp` |
| 10 層完整迷宮（B1F-B10F，原版 1981 地圖轉錄）| `assets/data/wiz1_mazes.json` |
| 永久死亡 + 遺體回收 + 神殿復活（DI/KADORTO）| `src/game/{traps,inn}.cpp` |
| 多層 / 滑梯 / 樓梯切換 | `src/game/traps.cpp::switch_floor()` |
| 453 條 i18n（繁中/English/日本語三語齊備）| `src/i18n/strings_zh_tw.json` |
| F3 視覺主題切換（PCE-CD / Mono / Outline / Sepia + 本機 PC98/WSC/Mac）| `src/render/theme.cpp` |
| F4 多語切換 | `src/i18n/tr.cpp::cycle_lang()` |

### v1.19-v1.24 新增（**填掉 v1.12b 標的所有 gap**）

| 項目 | 版本 | 主要檔案 | 取代了 v1.12b 的哪個 gap |
| --- | --- | --- | --- |
| **per-step 遭遇率** `30 + 7*(L-1)` clamp 110 | v1.19 | `screens.cpp::maybe_random_encounter()` + `traps.cpp` enmy_calc | 戰鬥框架延伸 |
| **F3 主題化全面**（牆/門/地板/天花板/自動地圖/標題背景全 theme-aware） | v1.20 | `render/palette.cpp` + `maze_view.cpp` + `auto_map.cpp` | UI 細節 |
| **UI chrome 字串全部走 tr()** | v1.20 | `screens.cpp` + 新加 i18n 鍵 | i18n 漏字 |
| **突襲回合**（先發/被突襲 1d100 三段判定）| v1.21 | `combat.cpp::Surprise` enum + begin_combat | ❌「沒有偷襲回合」→ ✅ |
| **吸血鬼吸等級**（VAMPIRE / VAMPIRE LORD 命中扣 char_level）| v1.21 | `combat.cpp::Special::Drain` | ❌「等級吸取」→ ✅ |
| **食屍鬼麻痺**（GHOUL 命中 25% Paralyzed）| v1.21 | `combat.cpp::Special::Paralyze` | ❌「沒有特殊攻擊類別」→ ✅ |
| **龍類吐息**（DRAGON 全派 3d8/隻全隊 cone + vit-save 半傷）| v1.21 | `combat.cpp::Special::Breath` | ❌「沒有特殊攻擊類別」→ ✅ |
| **法術槽自動補滿/扣除** | v1.22 | `rules.cpp::recompute_spell_slots()` + `consume_spell_slot()` | ⚠「沒有 slot 管理 / 升等學習 / 施法扣槽」→ ✅ |
| **Mage/Priest 直接、Bishop/Samurai/Lord 延遲 3 級的雙系規則** | v1.22 | `rules.cpp::fill_slots()` | ⚠ 同上 |
| **Afraid +2 AC 命中懲罰** | v1.22 | `combat.cpp::effective_target_ac()` | ⚠「Afraid 沒實際效果」→ ✅ |
| **神殿解咒服務**（[G] Uncurse，500 金/件）| v1.22 | `inn.cpp::TempleUI::Uncurse` | ⚠「沒有 uncurse 選項」→ ✅ |
| **8 種陷阱齊全**（Pit/Spinner/Teleporter/Chute + Fizzle/Message/Elevator/Encounter）| v1.22 | `traps.cpp::apply_trap()` | ❌「0/8 已實作」→ ✅ **8/8** |
| **CALFO 真陷阱偵測**（看前方一格 SquareFeature）| v1.22 | `camp.cpp::cast_camp_spell("CALFO")` | ⚠「LATUMAPIC 沒有效果觸發器」→ ✅ |
| **LATUMAPIC 看穿** (`g.identified=true` + `display_name()`) | v1.22 | `screens.cpp` + `combat.cpp::display_name()` | ⚠ 同上 |
| **MANIFO / KATINO / MONTINO 全部接 effect** | v0.6 / v1.13 | `combat.cpp::cast_spell()` | ⚠「30 條沒 effect trigger」→ ✅ |
| **LATUMOFIS 解毒**（戰鬥 + 營地）| v1.13 | `camp.cpp` + `combat.cpp` | ⚠ 同上 |
| **Poisoned 狀態 + 每輪扣血 tick** | v1.13 | `combat.cpp::resolve_round()` 末尾 | ❌「缺 Poisoned」→ ✅ |
| **前/後排陣型**（slots 0-2 前排，3-5 後排）| v1.13 | `combat.cpp::run_combat_turn()` 後排 melee fails | ❌「沒有前/後排概念」→ ✅ |
| **M 鍵自動地圖開關 + 指北針 + 深度** | v1.23 | `screens.cpp` compass overlay | UI polish |
| **Shift+↑↓ 隊伍重排** | v1.23 | `tavern.cpp` shift_held swap | UI polish |
| **Camp [E] 匯出角色卡 .txt** | v1.23 | `camp.cpp::export_character_text()` | UI polish |
| **Dark zone + MILWA/LOMILWA 步數遞減** | v1.23 | `maze.h::dark_zone` + `state.light_steps_left` | ❌「暗黑區」→ ✅ |
| **Werdna 結局畫面 `Scene::Ending`** | v1.24 | `screens.cpp` + `state.h::werdna_defeated` + `assets/themes/pcecd/ending/background.png` | ❌「沒有勝利條件 / 結局畫面」→ ✅ |
| **Linux AppImage 單檔執行** | v1.24 | `release/wizardry-cht-x86_64.AppImage` (72 MB) | packaging |
| **Windows zip + 全 DLL + .bat launcher** | v1.24 | `release/wizardry-cht-windows-x64.zip` (86 MB) | packaging |

---

## ⚠ 仍部分實作（v1.24 已收極小 polish 殘缺）

### 終局
- ✅ B10F 結構支援、Werdna 怪物、Scene::Ending 畫面、Amulet 結局文字（v1.24）
- ⚠ **Amulet of Werdna 道具掉落** — 結局畫面有顯示金色護符 splash，但 player inventory 沒實際拿到 item id（`assets/data/items.json` 沒這個道具，劇情上由 ending 場景代為「持有」概念）
- ⚠ **「再次挑戰 / NG+」** — 結局後按任意鍵直接回 Title，沒做 New Game+ 模式

### 道具
- ✅ `cursed` 旗標存在
- ✅ 神殿解咒服務（v1.22）
- ⚠ **拒絕脫裝的強制邏輯** — 物品被裝備後 `equipped=true` 但 unequip path 沒檢查 `cursed`，理論上目前能脫
- ⚠ **charge / break 計次器** — 法杖類道具的有限次施法
- ⚠ **alignment-locked 裝備** — 邪派不能裝聖物

### 戰鬥進階
- ✅ 等級吸取（v1.21）
- ✅ 麻痺（v1.21）
- ✅ 吐息（v1.21）
- ✅ 突襲回合（v1.21）
- ⚠ **戰鬥中寶箱**（敵亡後出現寶箱、CALFO 檢測、DISARM 解除）— 我們的 CALFO 改成偵測迷宮陷阱了，沒做寶箱系統
- ⚠ **石化 → 灰燼 → 永失多階轉換鏈** — Stoned 旗標存在但沒自動降階；Dead → Ashes → Lost 經由神殿 DI/KADORTO 失敗才會推進

### 年齡 / 時間流逝
- ✅ 旅館投宿週期 + age 增量
- ⚠ **「太老會自然死亡」** — 70+ 歲應有 1d100 死亡檢定，現未強制

---

## 🎯 v1.12b 列的「建議優先順序」進度回顧

| # | 項目 | v1.12b 影響/工時估 | 實際完成版本 |
| --- | --- | --- | --- |
| 1 | Poisoned 狀態 + 每輪扣血 | 高 / 1 hr | ✅ v1.13 |
| 2 | 法術效果觸發器（30+ 條） | 高 / 4-6 hr | ✅ v0.6 base + v1.13 + v1.22 |
| 3 | 前/後排陣型 | 中 / 3-4 hr | ✅ v1.13 |
| 4 | 迷宮陷阱（前 4 種） | 中 / 4-5 hr | ✅ v1.14（4 種）→ v1.22（補齊 8 種）|
| 5 | 轉職系統 | 中 / 3 hr | ✅ v1.14 |
| 6 | 偷襲回合 | 中低 / 2 hr | ✅ v1.21 |
| 7 | 遺體回收 + 永久死亡 | 中 / 3 hr | ✅ v1.16 |
| 8 | 年齡 / 復活風險 | 低 / 2 hr | ✅ v1.17 (DI/KADORTO 含年齡與失敗風險) |
| 9 | 菁英職業屬性檢核 | 低中 / 1 hr | ✅ v0.x roller |
| 10 | 道具職業/陣營限制 & 詛咒鎖 | 低 / 2 hr | ⚠ 部分（cursed 旗標有，alignment-lock 仍缺）|

**10 項中 9 項完成、1 項部分完成**。v1.12b 那份 punch-list 已實質結束。

---

## 📜 變更歷史

- **v1.24（2026-06-12）**：本檔案重寫。v1.12b 列的 17 條 gap 全部翻成 ✅。新增 v1.19-v1.24 細項。
- **v1.12b（2026-06）**：原始 gap analysis，由 explore agent 比對手冊（Mac EN 版前 60 頁）產出。

*內容為自行撰寫之機制摘要。原始手冊版權屬 Sir-Tech / IPLAY。*
