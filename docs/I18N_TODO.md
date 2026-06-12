# i18n TODO — 程式碼內仍硬編碼的英文字串

> 本文件**只列出**：`src/` 程式碼裡仍以 C++ 字串字面值（string literal）
> 直接寫死、未經 `tr()` 查表的「使用者可見英文字串」（user-facing strings）。
> 不修改程式碼，留給後續 i18n 維護處理。
>
> 不在此清單內的範疇：
> - 法術代號（HALITO / DIOS / ...）— 屬於遊戲規則 ID，按原版保留
> - 程式內部識別字（enum class、log key、JSON key 等）
> - DEBUG / log 訊息（只在開發階段顯示）
> - 鍵盤鍵名（SPACE / ESC / Enter / Tab）— 對應實體鍵盤標示，保留英文是慣例

最後一次掃描：2026-06-12（v1.24）

> **v1.20-v1.24 進度**：F3/F4 toast、標題提示、底部 status hint、戰鬥訊息
> （突襲/吸血/麻痺/吐息/法力不足/反魔法）、結局文字、神殿解咒、營地匯出
> ——這些原本 hardcoded 的字串 **全部已改走 `tr()`**，新增約 30 個 i18n 鍵，
> zh_TW / en / ja_JP 三語齊備。下列 TODO 表為 v1.18 時代殘餘，多為低優先級。

---

## A. 視窗標題列

| 檔案 | 行 | 字串 | 建議處理 |
|------|----|------|---------|
| `src/render/window.h` | 12 | `"Wizardry"` | 改為 `"Wizardry I — 巫術 I：瘋王試煉場"` 或經 `tr("window_title")` 查表 |

優先級：**低**。視窗標題只在桌面工作列短暫看見；多數玩家不會在意。
但若要全中文體驗，可加進 i18n 表並讓 SDL 視窗 init 時呼叫 `tr()`。

---

## B. 內建 demo 角色名（main.cpp 預設名冊）

| 檔案 | 行 | 字串 | 備註 |
|------|----|------|------|
| `src/main.cpp` | 45 | `"ALOZAR"` | 6 個預設角色之一 |
| `src/main.cpp` | 46 | `"ARYNN"` | |
| `src/main.cpp` | 47 | `"BRENDA"` | |
| `src/main.cpp` | 48 | `"CYRIL"` | |
| `src/main.cpp` | 49 | `"DRAEDA"` | |
| `src/main.cpp` | 50 | `"ELOWEN"` | |

優先級：**低**。原版 Wizardry I 內建的「預設拓荒小隊」名字本身就是
英文音譯人名，**保留原文可作為文化彩蛋**。如要做完全中文化，可以提供：
- ALOZAR → 「阿洛札」
- ARYNN → 「艾林」
- BRENDA → 「布蘭達」
- CYRIL → 「西里爾」
- DRAEDA → 「德蕾達」
- ELOWEN → 「艾洛文」

但**強烈建議走 i18n catalogue**而非直接改 main.cpp，
以維持「英文 ID + 中文顯示名」的設計。

---

## C. 混合中英文標籤

| 檔案 | 行 | 字串 | 評估 |
|------|----|------|------|
| `src/game/screens.cpp` | 77 | `"目前隊伍 / CURRENT PARTY"` | **保留**：刻意設計的雙語對照，方便玩家對應 1981 原版 UI |
| `src/game/screens.cpp` | 351 | `"自動繪圖 (Eye of Map)"` | **保留**：Eye of Map 是 v0.3 新增功能的官方稱呼 |

優先級：**無**（已是有意為之的設計）。

---

## D. 註解中的英文遊戲術語

`src/core/combat.cpp`、`src/game/camp.cpp` 等檔案的 spell 處理區，
有大量註解像 `// self heal lots`、`// cure poison` 等，
為**程式註解**（comment），不會顯示給玩家，**不在 i18n 範圍**。

但若未來想對程式碼做更友善的中文化，可考慮為每個 spell 加上中譯註解，
例如：

```cpp
"DIALMA",    // self heal lots — 大量自療
"MILWA",     // brief light — 短時照明
"LOMILWA",   // long light — 長時照明
"CALFO",     // identify trap (on chest) — 識破寶箱陷阱
"LATUMOFIS", // cure poison — 解毒
"DIALKO",    // cure paralysis/sleep — 解麻痺／沉睡
"DUMAPIC",   // show location — 顯示位置
```

優先級：**極低**（純註解，玩家看不到）。

---

## E. SDL 預設視窗 / 環境變數鍵

| 檔案 | 行 | 字串 | 說明 |
|------|----|------|------|
| `src/game/camp.cpp` | 344 | `std::getenv("HOME")` | POSIX 環境變數名，**不可翻譯** |

優先級：**無**（系統 API 名稱）。

---

## F. JSON i18n catalogue（已 100% 完成）

`src/i18n/strings_zh_tw.json` 在 2026-06-05 已達 **453/453 = 100% 翻譯率**。
原本剩餘的 40 條（多為 1981 年原版的版權聲明、磁片操作指示、罕用 UI 字串）
已全數補譯，含：
- COPYRIGHT (C) 1981 → 版權所有 (C) 1981 ...
- PLACE ORIGINAL MASTER SCENARIO SIDE IN → 請將原版正本場景面放入
- PUNCH WRITE-PROTECT NOTCH ON BOOT SIDE → 在開機面打防寫缺口
- 等等

對於 `SCENARIO.DATA` / `SCENARIO.MESGS` / `SYSTEM.PASCAL` 三個 UCSD Pascal
**檔名**，刻意保留原英文（檔名屬技術識別字，翻譯反而會誤導讀者）。

---

## 維護建議

下次 review 時：

1. 跑 `grep -rn --include='*.cpp' -E '"[A-Z][a-zA-Z ]{3,}"' src/` 找新增的英文字串
2. 凡是 `draw_text()` / `draw_line()` 第三參數出現英文 → 轉 `tr()`
3. 新加的 enum 對外顯示名 → 進 `strings_zh_tw.json`
4. JSON 新增 key 後務必跑 `python3 -c "import json; ..."` 驗證 zh_TW 已填寫

---

**結論**：截至 2026-06-05，本專案的玩家可見字串 i18n 覆蓋率為 **100%**。
僅剩的英文字串均屬「程式內部識別字 / 視窗標題 / 雙語對照刻意保留」三類，
不影響繁中遊戲體驗。
