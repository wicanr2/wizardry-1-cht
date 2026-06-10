---
layout: default
title: 手冊對照表 — Wizardry I 規則書 vs 現有實作 Gap Analysis
---

# 手冊對照表 — Wizardry I 規則書 vs 現有實作

> 這份文件比對 1981 年 Sir-Tech 原始手冊（Mac EN 重排版，254 頁
> 合集中前 ~60 頁為 W1 章節）與 `wizardry-cht` 現有實作的落差，
> 用來決定後續實作優先順序。所有「規則」描述為自行改寫，
> 不直接引用手冊原文。

最後更新：v1.12b

---

## ✅ 已完成

| 系統 | 狀態 | 主要檔案 |
| --- | --- | --- |
| 5 種族 / 3 陣營 / 8 職業列舉 | ✅ | `src/core/character.h` |
| 6 屬性骰點（DERF roller） | ✅ | `src/game/roller.cpp` |
| 51 道法術完整 catalogue | ✅ | `assets/data/spells.json` |
| 多群組怪物戰鬥框架 | ✅ | `src/core/combat.{h,cpp}` |
| 4 種戰鬥動作（攻擊/施法/防禦/逃跑） | ✅ | `combat.h` `PlayerAction` |
| 識別/未識別怪物欄位 | ✅ | `CombatGroup::identified` |
| 8 格背包 + equipped/identified/cursed 三旗標 | ✅ | `core/character.h` |
| 城堡 5 項服務（酒館/旅館/商店/神殿/培訓場）UI | ✅ | `src/game/{tavern,inn,shop,...}` |
| 旅館 5 級房型 | ✅ | `src/game/inn.cpp` |
| 多 slot 存檔（v1.11b） | ✅ | `src/save/gamesave.cpp` |

---

## ⚠ 部分實作

### 角色 & 職業
- **種族屬性修正** — `Race` enum 存在，但 roller 沒套用 race-stat 修正
- **陣營/職業限制** — Lord 須 Good、Ninja 須 Evil、Priest 不可 Neutral —— roller 中未檢核
- **菁英職業屬性門檻** — Bishop / Samurai / Lord / Ninja 的最低屬性（手冊 + snafaru v3.2 校正版）未強制
- **轉職系統** — 達等級門檻可轉職、保留 HP — 缺整個機制

### 法術系統
- 51 條 catalogue 在 JSON，但 **約 30 條沒有 effect trigger**：
  - `KATINO`（睡眠）、`MANIFO`（麻痺）、`MONTINO`（沉默）
  - `LATUMAPIC`（識別怪物名）、`LATUMOFIS`（解毒）
  - `MALOR`（瞬移）、`LOKTOFEIT`（傳送回城）
  - `DI / KADORTO`（復活，有失敗率）、`MAHAMAN`（7 種隨機效果）
- 角色 struct 有 `mage_spell_slots[7]` / `priest_spell_slots[7]` 陣列，但 **沒有 slot 管理 / 升等學習 / 施法扣槽**
- 多數法術標記 `camp_only` 但沒有 camp 施法 UI 路徑（v1.11 已加部分）

### 戰鬥
- 多群組支援存在，但 **沒有前/後排概念**（手冊：只有前 3 人能近戰）
- **沒有先攻 / 行動順序**
- **沒有偷襲回合**（手冊：偷襲時隊伍免費攻擊一輪、施法者該輪不能放）
- **沒有特殊攻擊類別**（麻痺/中毒/吸 level/石化/暴擊）

### 狀態
- `Status` enum 有 8 種：Ok/Afraid/Asleep/Paralyzed/Stoned/Dead/Ashes/Lost
- **缺 `Poisoned`** — 手冊多處提及，但 enum 沒這欄位
- Afraid / Asleep / Paralyzed 旗標有但 **沒實際效果**（如：Afraid 應降低命中、Paralyzed 應跳過行動）

### 城堡服務
- 商店有 buy/sell/identify，**沒有 uncurse 選項**
- 神殿有治療，**復活的 cost scaling 與年齡推進缺**
- 旅館 5 房型存在，**沒有「休息→升等→年齡上升」的時間流逝機制**

### 道具
- `cursed` 旗標存在，但 **拒絕脫裝的強制邏輯沒接**
- **沒有 charge / break** 計次器
- **沒有 alignment-locked 裝備檢查**
- **沒有職業限定武器** lookup（手冊：盜賊只能用短劍/匕首）

### 終局
- B10F 結構支援、Werdna 怪物與護身符道具都在 catalogue
- **沒有護身符的 cursed-block-unequip 處理**
- **沒有勝利條件 / 結局畫面**

---

## ❌ 未實作

### 迷宮特殊地塊
0 / 8 已實作：
1. 陷阱坑（HP 直接扣）
2. 滑梯（強制掉下一層）
3. 旋轉地塊（隨機改變朝向）
4. 傳送地塊
5. 阻擋地塊（耽誤一輪）
6. 反魔法地塊（暫時禁用法術）
7. 卡住的門（需破壞才能通過）
8. 訊息地塊（牆上塗鴉、文字事件）

### 暗黑區 & 隱藏元素
- 黑暗區（`MILWA / LOMILWA` 點燈才看見）
- 隱藏門（`MILWA` 可揭露）
- 反魔法區（`MAPORFIC` 護盾失效）

### 角色生命週期
- **永久死亡** — 死後若沒成功復活則永久消失，現在 Status::Dead 只是旗標
- **遺體回收** — 隊伍全滅後屍體留在迷宮原格，要新隊伍走過去拾回
- **年齡推進 & 復活風險** — 休息/復活/某些法術都推進年齡，太老會自然死亡

### 戰鬥進階
- 戰鬥中寶箱與陷阱（`CALFO` 檢測、`DISARM` 解除）
- 等級吸取（vampire 類怪物）
- 石化 → 灰燼 → 永失的多階轉換鏈

---

## 🎯 建議優先順序（影響/工時）

| # | 項目 | 影響 | 工時估計 |
| --- | --- | --- | --- |
| 1 | **Poisoned 狀態 + 每輪扣血** | 高（手冊基本款） | ~1 hr |
| 2 | **法術效果觸發器**（睡眠/麻痺/沉默/識別/解毒） | 高（30+ 法術從「能選但沒作用」變真實） | 4–6 hr |
| 3 | **前/後排陣型**（前 3 人才能近戰） | 中（戰術深度） | 3–4 hr |
| 4 | **迷宮陷阱（前 4 種：坑/滑梯/旋轉/傳送）** | 中（迷宮探索感） | 4–5 hr |
| 5 | **轉職系統** | 中（核心 RPG 體驗） | 3 hr |
| 6 | **偷襲回合** | 中低 | 2 hr |
| 7 | **遺體回收 + 永久死亡** | 中（標誌性難度） | 3 hr |
| 8 | **年齡 / 復活風險** | 低（細節） | 2 hr |
| 9 | **菁英職業屬性檢核** | 低中（已半實作） | 1 hr |
| 10 | **道具職業/陣營限制 & 詛咒鎖** | 低 | 2 hr |

頭 3 項做完，遊戲體驗就會從「能打」升級到「跟 1981 規則對齊」。

---

*Gap analysis 由 v1.12b 後 explore agent 比對手冊（Mac EN 版前 60 頁）
產出，內容為自行撰寫之機制摘要。原始手冊版權屬 Sir-Tech / IPLAY。*
