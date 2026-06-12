---
layout: default
title: 瘋王試煉場攻略 — B1F〜B10F 完整迷宮指南
---

# 瘋王試煉場攻略 — B1F〜B10F 完整迷宮指南

> **為什麼讀這篇？**
> Wizardry I 沒有 NPC、沒有任務日誌、沒有指引箭頭。
> 你在 1981 年買到磁碟時，得到的是**一張方格紙、一支鉛筆、跟一片黑底封面手冊**。
> 所有迷宮資訊都得**自己畫地圖**——這是 Wizardry 的精髓，也是它的詛咒。
> 這篇是 2026 年的「**作弊地圖**」：用社群 45 年累積的攻略知識，
> 把每層迷宮的**重點路線**、**Boss 戰**、**隱藏寶藏**、**陷阱位置**用中文整理出來。
>
> **使用建議**：第一遍**不要看**這篇——自己畫地圖才是 Wizardry 的浪漫。
> 但如果你在 B4 卡了 3 小時找不到下樓樓梯（這是 1980 年代台灣玩家共同回憶），
> 那這篇就是你的救命稻草。

---

## 目錄

1. [通用知識：座標系統與機制](#general)
2. [B1F — 試煉之初與 Murphy's Ghost（墨菲之鬼）經驗農場](#b1f)
3. [B2F — 旋轉陷阱的洗禮](#b2f)
4. [B3F — 暗門密集區](#b3f)
5. [B4F — 軍隊撤退的邊界](#b4f)
6. [B5F — 食人魔與滑道](#b5f)
7. [B6F — 開始遭遇施法者](#b6f)
8. [B7F — 第一個真正的強敵](#b7f)
9. [B8F — 巨人與吸血鬼地獄](#b8f)
10. [B9F — 倒數第二層的絕望](#b9f)
11. [B10F — Contra Dextra Avenue 與 Werdna 戰](#b10f)
12. [回程：護身符的禁忌](#return)
13. [引用來源](#sources)

---

<a name="general"></a>
## 一、通用知識：座標系統與機制

### Dumapic 座標

施放 **DUMAPIC**（魔法師 1 級咒語）會顯示目前位置：

```
You are on level 3,
3 squares North, 8 squares East,
facing North.
```

社群統一寫法是 **`3N-8E`**（北 3 格、東 8 格）。
**所有層的 `0N-0E` 都是該層第一次踩上來的「起始格」**——通常是樓梯口。

### Wrap-around（迷宮環繞）

每層都是 **20×20**，但**邊界會環繞**：

- 從 `5N-19E` 往東走一步 → 變成 `5N-0E`
- 從 `19N-5E` 往北走一步 → 變成 `0N-5E`

**這個機制讓「扶牆走」失效**——你可能繞了整個 20×20 還回不到起點。

### 八種陷阱（本專案 v1.22 全數實作完成）

| 陷阱 | 中文 | 效果 | 本專案實作 |
|------|------|------|------|
| Pit | 坑洞 | 全隊每人 -1d8 HP | ✅ `traps.cpp::pit_damage()` |
| Chute | 滑道 | **強制掉到下一層**（無法回頭！） | ✅ `chute()` + `switch_floor()` |
| Spinner | 旋轉盤 | 隨機改變朝向，**地圖完全失準** | ✅ `spinner()` 隨機 facing |
| Teleporter | 傳送器 | 傳送到該層或他層隨機位置 | ✅ `teleporter()` |
| Encounter | 埋伏格 | **強制觸發戰鬥**（不論機率） | ✅ `force_encounter()` (v1.22) |
| Message | 石刻 | **留言型情境描述** | ✅ `message_square()` 6 條範本 (v1.22) |
| Elevator | 電梯 | **跳到 B4-B9 區間** | ✅ `elevator()` (v1.22) |
| Fizzle | 靜咒區 | **暫時禁止施法** | ✅ `anti_magic()` + `state.anti_magic_here` (v1.22) |

### 推薦小隊配置

| 位置 | 職業 | 任務 |
|------|------|------|
| 1（前排） | 戰士 / 武士 | 主攻 |
| 2（前排） | 戰士 / 領主 | 副攻 + 治療備援 |
| 3（前排） | 忍者 / 戰士 | 高速攻擊 |
| 4（後排） | 主教 | 鑑定 + 雙系法術 |
| 5（後排） | 魔法師 | 群攻法術 |
| 6（後排） | 牧師 | 治療與輔助 |

> **黃金組合**：F + F + S + B + M + P
> （戰士 + 戰士 + 武士 + 主教 + 法師 + 牧師）

---

<a name="b1f"></a>
## 二、B1F — 試煉之初與 Murphy's Ghost（墨菲之鬼）經驗農場

> **進入提示**：從城堡邊緣（Edge of Town）選 `M` 進入迷宮，**第一層就是 B1F**。
> 起始位置 `0N-0E`，面朝北。

### 路線概要

B1F 是新手村——大部分敵人是 **Bubbly Slime（泡泡黏液）**、**Orc（獸人）**、**Kobold（狗頭人）**，
傷害低、HP 少、不會吸等級。但**不要小看 B1F**——它有一個專屬機制讓老玩家又愛又恨。

### Murphy's Ghost（墨菲之鬼）經驗農場

> 這是 Wizardry 系列史上**最有名的設計失誤**之一，後來被官方在重製版**繼承保留**。

**位置**：B1F 的 `5N-13E`，一個鑲滿寶石的兜帽人雕像。

**機制**：靠近雕像 → 按 `SEARCH` → 觸發戰鬥 → 出現 **Murphy's Ghost（墨菲之鬼）**。

| Murphy's Ghost（墨菲之鬼）資料 | 數值 |
|---|---|
| HP | 20–110（1d100 級別） |
| AC | **-3**（**對 1 級小隊難以命中**） |
| 傷害 | **1d1+1 = 2 點** |
| XP | **4,450** |
| 抗性 | 火/冰 40%，**對死亡魔法 100% 免疫** |
| 弱點 | **可被 KATINO 催眠** |
| 重生率 | 打死後 **80% 機率再次出現** |

**為何叫經驗農場**：

- 它**幾乎不傷你**（每回合 2 點）
- 給的 **XP 比 B1 任何怪物都多 80 倍**
- **80% 機率重生** → 同一格反覆刷
- 1 級小隊熬一晚可以練到 **5–6 級**

**推薦戰術**：

1. 進場第一步：**牧師 PORFIC** + **MILWA**（自身 AC -4 + 照明）
2. **法師 KATINO** 把它睡掉
3. 戰士兩人**狂砍**（睡眠中敵人 AC 變 10，**必中**）
4. 牧師備用 **DIOS** 治療
5. 牧師另一動 **MATU**（全隊 AC -2）

> **譯註**：「Murphy's Ghost」直譯「莫菲的鬼魂」——
> 這個 Murphy 不是英文「莫非定律」的那個，
> 而是當年 Greenberg 與 Woodhead **大學室友 Murphy** 的名字。

### B1F 出口

樓梯下 B2F 位置在 **`8N-8E`** 附近的房間——**從起點往北 8 格，往東 8 格**。
中間會穿過幾個小房間，**從 `2N-9W`** 過一道隱藏門。

---

<a name="b2f"></a>
## 三、B2F — 旋轉陷阱的洗禮

> **進入提示**：起始 `12E-7N`，面朝你進入的方向。
> 這層的怪物等級 2–3，主要是 **Rogue（惡棍）**、**Bushwhacker（伏擊者）**、
> **Highwayman（攔路強盜）**——**會偷錢但不致命**。

### 第一個 Spinner

B2F 是玩家第一次遇到 **Spinner（旋轉盤）** 的層數。它的可怕在於：

- **無視覺提示**——你踩上去後地圖完全沒提示
- **轉向是隨機的**——可能轉 90°、180°、270°
- **配合 wrap-around** 你可以走 10 步就完全迷路

**對應**：**每走一步施 DUMAPIC**（魔法師 1 級）即時更新座標，
或裝備 **AMULET OF JEWELS**（5,000 GP，5% 機率自動施 DUMAPIC）。

### Creeping Coins 寶藏房

> *「they have like.. oh maybe 2 Hitpoints each」*
> — Kelly R. Flewin 攻略

在 B2F 某個房間（座標各版本不同，請以 DUMAPIC 為準）會遇到**一群 Creeping Coins（活幣）**——
看起來像會動的金幣堆，HP **只有 2 點**，但**一隊有 9 隻**，**總 XP 約 2,900**。
**是 B2F 的小型 Murphy's Ghost（墨菲之鬼）替代品**。

### B2F 出口

樓梯到 B3F 在**北邊深處**，需要通過 2 道隱藏門。

---

<a name="b3f"></a>
## 四、B3F — 暗門密集區

> **進入提示**：起始 `5N-5E` 附近。
> 怪物：**Zombie（喪屍）**、**Coyote（郊狼）**、**Highwayman（攔路強盜）**、開始出現 **Gas Cloud（毒氣雲）**。

### 暗門特性

B3F 是「**全層 30% 牆其實是隱藏門**」的層。**MILWA / LOMILWA** 照明咒語
讓暗門顯示為「點線」——**沒有照明你會錯過 80% 的路線**。

**裝備推薦**：
- **AMULET OF DUMAPIC**（部分版本）讓座標常駐顯示
- **每進新層必先 LOMILWA**（牧師 3 級，效果 10000 步）

### 第一個強制戰鬥（Encounter Square）

B3F 有 2–3 個格子是**強制觸發戰鬥**（踩到就打）。如果隊伍不夠強，**逃跑機率約 60%**——
逃跑失敗 = 全隊被打。

### B3F 出口

樓梯到 B4F 隱藏在**西北角的房間**，需穿過一道**暗門 + 一個傳送器**。

---

<a name="b4f"></a>
## 五、B4F — 軍隊撤退的邊界

> **進入提示**：起始 `1N-1E`。
> **B4F 是 Trebor 軍隊清剿的最後一層**——故事上有重大意義。
> 怪物開始出現 **Lvl 3 Mage**、**Lvl 3 Priest**、**Giant Toad（巨蟾蜍）**、**Ogre（食人魔）**。

### 警報觸發點

B4F 有一個**無法避免**的事件格——踩上去後，**怪物被分配中心會強化所有後續遭遇**。
這是 Trebor 軍隊撤退後**設置的警報系統**。

### Blue Ribbon（藍緞帶）

> **這是 B4F 最重要的事件**。

某個房間有個 **Statue of Bear（熊像）**——`SEARCH` 後給你一段謎語，
解開後**獲得 Blue Ribbon（藍緞帶）**。

**Blue Ribbon 的用途**：B4F 有個**電梯**（Elevator），按下後直通 B9F——
**但沒有藍緞帶你按不動**。這是 Wizardry 第一個「**鑰匙道具**」設計。

### 推薦做法

1. B4F **第一次來** → 練到 5–6 級 → 拿藍緞帶
2. **不**搭電梯下 B9F（太弱）→ **回城**休息
3. B5F–B8F **正常爬** → 升到 8–9 級
4. **重回 B4F 搭電梯** → 跳過 B9F 直接 B10F（部分版本可行）

### B4F 出口

- 樓梯到 B5F 在**東邊深處**
- **電梯到 B9F** 在**西邊房間**（需 Blue Ribbon）

---

<a name="b5f"></a>
## 六、B5F — 食人魔與滑道

> **進入提示**：起始 `0N-0E`。
> 怪物：**Giant Toad、Ogre、Were Bear（人熊）、Lvl 4 Mage / Priest**。

### Chute 陷阱

B5F 是**第一個有 Chute（滑道）的層**。Chute 會**強制把你掉到 B6F 的隨機位置**——
**你準備不足時掉下去 = 死定了**。

**辨識方法**：除非用 CALFO（牧師 2 級）鑑定，否則 **Chute 看起來跟普通格沒兩樣**。
**唯一防範**：每進新格前先讓盜賊用 `INSPECT` 檢查。

### 出口

樓梯到 B6F 在**南邊**，但通常玩家會**直接踩 Chute 跳過 B5 探索**——
**這其實是 Greenberg 故意設計的「快通道」**。

---

<a name="b6f"></a>
## 七、B6F — 開始遭遇施法者

> **進入提示**：起始位置取決於你怎麼下來——爬樓梯 `0N-0E`，掉 Chute 則隨機。
> 怪物：**Lvl 4 Mage、Lvl 5 Priest、Bishop（主教）、Gas Dragon（毒氣龍）**。

### 戰術轉變

從 B6F 開始，怪物會**主動施法**：

- **MAHALITO**（4d6 火球，群傷）
- **DALTO**（6d6 冰錐，群傷）
- **KATINO**（**讓你全隊睡眠**——睡眠中被攻擊必中）

**對應**：**牧師必背 MONTINO**（2 級，讓敵群靜默）。
這個咒語在 v3.2 修正前**有 bug 會失敗**，本專案 v0.6 已修。

### 寶藏

B6F 開始出現 **+1 武器/防具** 的寶箱。**請務必讓盜賊 INSPECT 寶箱**——
B6F 之後的寶箱**陷阱會殺死整隊**（**Teleporter / Anti-Mage / Stunner / Mage Blaster**）。

---

<a name="b7f"></a>
## 八、B7F — 第一個真正的強敵

> **進入提示**：起始 `0N-0E`。
> 怪物：**Bishop（主教）**、**Gas Dragon（毒氣龍）**、**Arch Mage（大魔導師）**、**High Ninja（高階忍者）**。

### Arch Mage（大魔導師）警告

| Arch Mage（大魔導師） | 數值 |
|---|---|
| HP | 7d8 ≈ 28 |
| AC | -2 |
| XP | 15,880（等同 Werdna！） |
| 法術 | **TILTOWAIT 可用**（10d15 = 期望值 80 點全隊群傷） |

**處理方式**：**第一回合 KATINO 必睡，或 MONTINO 必靜默**。**讓他施一次 TILTOWAIT = 全隊滅團**。

### 寶藏

B7F 開始出現 **+2 武器**、**Staff of Mogref**、**Mage Masher**（對法師額外傷害）。

---

<a name="b8f"></a>
## 九、B8F — 巨人與吸血鬼地獄

> **進入提示**：起始隨機（多個傳送點）。
> 怪物：**Frost Giant（霜巨人）、Fire Giant（火巨人）、Vampire（吸血鬼）、Dragon Zombie（龍喪屍）**。

### Vampire 警告 ⚠️

> *「AVOID VAMPIRES! ...they can drain you 1-2 levels」*
> — Flewin

| Vampire | 數值 |
|---|---|
| HP | 10d8 ≈ 45 |
| AC | **0** |
| XP | 29,400 |
| 特殊 | **每擊吸 1–2 等級**（無條件、無抗性） |

**等級流失 = 永久損失**。被吸 2 級 = **重練 50% 的角色經驗值**。

**對應**：
- 戰士裝備 **+2 板甲 + 大盾** → AC 接近 -10 → Vampire 命中率掉很低
- 牧師備用 **ZILWAN**（魔法師 6 級，**對單個不死生物 10d200 = 期望值 1010 傷害，秒殺**）

### Giant 推薦

霜巨人、火巨人傷害高（每擊 8–15 點）但**不吸等級**——比 Vampire 友善多了。

---

<a name="b9f"></a>
## 十、B9F — 倒數第二層的絕望

> **進入提示**：起始 `0N-0E`，或從 B4F 電梯來。
> 怪物：**Lifestealer（奪命幽魂）、Master Ninja（忍者大師）、Dragon Zombie、Will-o-Wisp**。

### Lifestealer 警告 ⚠️⚠️

| Lifestealer | 數值 |
|---|---|
| HP | 13d8 ≈ 58 |
| AC | **-1** |
| XP | 35,200 |
| 特殊 | **每擊吸 1–3 級**（比 Vampire 還狠）+ 高機率麻痺 |

**這是全遊戲最讓人崩潰的怪物**。

### Will-o-Wisp（鬼火）

| Will-o-Wisp | 數值 |
|---|---|
| HP | 4d8 ≈ 18（**低**） |
| AC | **-8**（**全遊戲最高難命中**） |
| XP | 42,880（**單隻 4 萬 2，跟 Werdna 同級**） |

它就是 **B9F 的 Murphy's Ghost（墨菲之鬼）**——傷害低但**幾乎打不到**，可以靠運氣賺大量 XP。

### B9F 滑道捷徑

B9F **西邊 1 格** 有個 **Chute** 直通 **B10F**——
這是**通往 Werdna 的最短路徑**，但你會掉到 B10F **隨機位置**。

### B9F 出口

- 樓梯到 B10F 在**北邊深處**
- **Chute 在西邊 `0N-1W`**

---

<a name="b10f"></a>
## 十一、B10F — Contra Dextra Avenue 與 Werdna 戰

> **進入提示**：起始 `0N-0E`。**這是最終層**。
> 全層被分成 **7 個獨立房間**，**只能用傳送門連接**。

### Contra Dextra Avenue 暗語

迷宮入口有一塊**石碑**寫著：

> *「CONTRA DEXTRA AVENUE」*

這是**拉丁文 + 法文混搭**，意思是：

- **CONTRA** = 反、相反
- **DEXTRA** = 右
- **AVENUE** = 道路

**整句翻譯**：「**走右邊的反面**」=「**全部走左邊**」。

> **這是通關 Werdna 的核心提示**：
> **每次選擇門時，**永遠選左**——選右會被傳送回起點，選左會前進。**

### 三種傳送門

| 類型 | 位置 | 效果 |
|------|------|------|
| **回城** | 起點往右 1 格 | **直接傳送回 Trebor 城堡** |
| **前進** | 每個房間的左邊出口 | 進入下一個 chamber |
| **重置** | 每個房間的右邊出口 | **回到第一個 chamber** |

### 6 個守門人戰鬥

從第 1 個 chamber 到 Werdna 房，**必須打贏 6 場守門人戰**：

| Chamber | 守門人推測組合 |
|---|---|
| 1 | 4 × Master Ninja |
| 2 | 4 × Greater Demon |
| 3 | 4 × Vampire Lord |
| 4 | 4 × High Ninja + 2 × Fire Giant |
| 5 | 4 × Arch Mage |
| 6 | 4 × Lifestealer + 1 × Wisp |

> 各版本順序不同，但**強度遞增**。
> **6 場連戰 + 中間不能回城**——必須**事先準備滿血/滿 MP/復活卷**。

### Werdna（沃登納）戰

**第 7 chamber = Werdna's Lair（沃登納巢穴）**。

#### 敵人

- **Werdna（沃登納，瘋王）**（HP 60、AC -7、能施 TILTOWAIT / MAHAMAN / MAKANITO）
- **Vampire Lord（吸血鬼領主）** × 1
- **Vampire（吸血鬼）** × 4

**對戰流程**：

**回合 1**（**Initiative 必勝**——使用 Dagger of Speed 等高速武器）：

1. 法師：**MONTINO**（讓 Werdna 靜默，**不能施 TILTOWAIT**）
2. 牧師 A：**ZILWAN** → 秒殺一個 Vampire
3. 牧師 B：**MABADI**（讓 Vampire Lord HP 剩 1d8）
4. 戰士：全部砍 Werdna
5. 武士：砍 Werdna
6. 忍者：砍 Werdna（**5% 機率直接斬首**）

**回合 2**：

1. 法師：**TILTOWAIT**（10d15 = 80 點全敵群傷，**清掉所有 Vampire**）
2. 牧師：**MADI**（全隊滿血）
3. 戰士群：繼續砍 Werdna（**HP 60，2 個戰士 +2 攻擊一回合可以打掉**）

> **Werdna 死亡訊息**：
> *「YOU HAVE DEFEATED WERDNA! HE DROPS THE AMULET!」*
> 「你擊敗了沃登納！他掉落了護身符！」

---

<a name="return"></a>
## 十二、回程：護身符的禁忌

打死 Werdna 後，你會得到 **WERDNA'S AMULET（沃登納護身符）**。

### ⚠️ 千萬不要裝備 ⚠️

護身符是**詛咒道具**：
- **AC -20**（聽起來無敵）
- **每回合 +5 HP**（聽起來無敵）
- **詛咒：裝上去脫不下來**
- **只有付 1,000,000 GP 給 Boltac 才能解除**

### 正確用法

- 不裝備，**放在背包**
- 隊伍每次戰鬥前**手傳給瀕死隊員**用 → **5 HP/回合的免費治療**
- **不傳給同一個人超過 1 戰** → 避免被詛咒鎖死

### 回城

第 7 chamber **打死 Werdna 後**，會出現一個**選項**：
**「DO YOU WISH TO BE TELEPORTED BACK TO THE CASTLE?」**

選 **YES** → **直接回城** → 結局。

> 你會看到字幕：
> *「YOU HAVE COMPLETED THE PROVING GROUNDS!」*
> 「你完成了瘋王試煉場！」
>
> 然後城堡裡 Trebor 會給你封爵 ——
> 但別忘了 ——
> **下面那 9 層的試煉場還在開放，新一批冒險者明天就會進去。**
>
> *Wizardry I 的結局是：故事**沒有真正結束**。*

---

<a name="sources"></a>
## 十三、引用來源

- [StrategyWiki — Wizardry 1 Walkthrough](https://strategywiki.org/wiki/Wizardry:_Proving_Grounds_of_the_Mad_Overlord/Walkthrough)
- [Wizardry Wiki (wiki.gg) — Murphy's Ghost](https://wizardry.wiki.gg/wiki/Murphy%27s_Ghost)
- [Wizardry Wiki (fandom) — Walkthrough](https://wizardry.fandom.com/wiki/Wizardry:_Proving_Grounds_of_the_Mad_Overlord_walkthrough)
- [Kelly R. Flewin — Wizardry 1 Walkthrough（Snafaru 鏡像）](https://www.zimlab.com/wizardry/walk/w1/1/wizardry-1-walkthrough-1.htm)
- [GameFAQs — Kelly R. Flewin Wizardry 1 Guide](https://gamefaqs.gamespot.com/pc/564837-wizardry-proving-grounds-of-the-mad-overlord/faqs/7432)
- [DungeonCrawl-Classics — A Proving Grounds Guide](https://dungeoncrawl-classics.com/wizardry-series/proving-grounds-of-the-mad-overlord/guide/)
- [The-Spoiler — Wizardry 1 Walkthrough](https://the-spoiler.com/RPG/Sir-Tech/wizardry.1.2.html)
- [LP Archive — Wizardry 1 The Gauntlet](https://lparchive.org/Wizardry-Proving-Grounds-of-the-Mad-Overlord/Update%2008/)
- [CRPG Adventures — Wizardry Level 10](http://crpgadventures.blogspot.com/2021/09/wizardry-level-10.html)
- 本專案 `assets/data/monsters.json`（怪物 stat 數值）
- Sir-tech《Ultimate Wizardry Archives》Manual, 1998 Interplay 再版, p18, p33–36
