---
layout: default
title: Wizardry 1 攻略指南 — 從新角到通關 Werdna
---

# Wizardry 1 攻略指南 — 從新角到通關 Werdna

> **這篇是給誰看的？**
> 給那群在 1985 年買到 Apple II 磁碟、第一次踏進 **TREBOR 城堡** 就把整隊 6 個 1 級小屁孩送進 B2F、
> 結果被一群 *Creeping Coins* 圍毆到全滅、隔天在學校跟同學交換《軟體世界》攻略影本的人。
> 給那群 2026 年下載這個重製版、想說「我來看看當年到底有多硬」、然後在 B4F 卡了 3 小時的人。
>
> **跟 [WALKTHROUGH.md](WALKTHROUGH.md) 的差別：**
> Walkthrough 是「**每層怎麼走**」——地圖、Boss 位置、樓梯出口。
> 這篇是「**怎麼玩才不會死**」——開局 roller、職業搭配、咒語優先級、五大不要踩的坑。
> 兩篇互補：先讀這篇學戰術，再翻 Walkthrough 找路線。
>
> **使用建議**：第一次玩請**先存 5 個槽**。Wizardry 1 沒有 game over 提示——
> 你的隊伍可能在你以為「還行」的時候已經死光了。

---

## 目錄

1. [一、開局選擇 — Character Roller 戰略](#opening)
2. [二、前期 B1F-B3F — 從 0 到 5 級](#early)
3. [三、中期 B4F-B6F — 5 到 10 級的鴻溝](#mid)
4. [四、後期 B7F-B9F — 10 到 13 級的等級流失地獄](#late)
5. [五、終戰 — B10F 與 沃登納（Werdna）](#endgame)
6. [六、八職業專業攻略](#classes)
7. [七、51 咒語優先級表](#spells)
8. [八、五大「不要踩」陷阱](#pitfalls)
9. [九、本專案實作差異速查（v0.x-v1.24）](#implementation)
10. [十、引用來源](#sources)

---

<a name="opening"></a>
## 一、開局選擇 — Character Roller 戰略

> 1981 年 Sir-tech 的 Andrew Greenberg 把 roller 設計成「**第一關就是 RNG 試煉**」——
> 還沒進迷宮你就得先跟 random number generator 較勁。
> 1985 年的台灣玩家為了 reroll 出 18+ bonus point，可以在 Apple II 前面坐一整個下午——
> 那時候沒有 quick save，**reroll = 重打名字 + 重選種族 + 重看 5 秒動畫**。
> 本專案保留了完整的 1981 規則，但 reroll 流程改成**按一個鍵就重骰**——
> 已經是 2026 年了，老玩家的時間值錢。

### 1.1 五種族屬性對齊（含本專案 `rules.cpp::base_attr()` 實際數值）

| 種族 | STR | IQ | PIE | VIT | AGI | LUC | 最適合職業 |
|------|-----|-----|-----|-----|-----|-----|-----------|
| **Human（人類）** | 8 | 8 | 5 | 8 | 8 | 9 | 全能型，**Lord 主修**（PIE 起始低需補） |
| **Elf（精靈）** | 7 | 10 | 10 | 6 | 9 | 6 | **Mage / Bishop / Priest** 首選（雙 10 起跳） |
| **Dwarf（矮人）** | 10 | 7 | 10 | 10 | 5 | 6 | **Fighter / Priest / Lord**（VIT 10 HP 池厚） |
| **Gnome（地侏）** | 7 | 7 | 10 | 8 | 10 | 7 | **Priest / Bishop / Thief**（PIE+AGI 雙 10） |
| **Hobbit（哈比）** | 5 | 7 | 7 | 6 | 10 | 12 | **Thief / Ninja**（LUC 12 全遊戲最高，盜寶神族） |

> **老玩家共識**：**精靈法師 + 矮人戰士 + 哈比盜賊** 是「**三族鐵三角**」。
> 1985 年《軟體世界》攻略本第一篇講的就是這組。
>
> 數值來源：`src/core/rules.cpp::base_attr()`，1:1 對應 Sir-tech 1981 原版。

### 1.2 六屬性優先級 + 最低門檻

| 屬性 | 對誰重要 | 最低門檻 |
|------|---------|---------|
| **STR 力量** | Fighter / Samurai / Lord / Ninja | **15+** 才能走 Samurai/Lord/Ninja |
| **IQ 智力** | Mage / Bishop / Samurai / Ninja | Bishop 12+, Samurai 11+, **Ninja 15+** |
| **PIE 信仰** | Priest / Bishop / Lord / Ninja | Lord 12+, **Ninja 15+** |
| **VIT 體力** | **所有人**（HP 上限 + 復活成功率） | 16+ 開始每級 HP 加成（`rules.cpp::vit_bonus`） |
| **AGI 敏捷** | Thief / Ninja / 後排施法者 | **Lord 14+**, Ninja 15+ |
| **LUC 幸運** | 全員（影響 saving throw、寶箱解陷阱） | **Lord 15+**, Ninja 15+ |

> **「VIT 16 是隱形分水嶺」**——`rules.cpp:45` 寫死了 `vit_bonus = (vitality >= 16) ? (vitality - 15) : 0`，
> 也就是 VIT 16 每級 HP +1，VIT 18 每級 HP +3。**戰士 VIT 16 vs VIT 18，13 級時 HP 差 26 點**——
> 這就是滅團與沒滅團的差別。

### 1.3 Bonus Point 配點策略

本專案 `rules.cpp::roll_bonus_points()` 的演算法：

```cpp
int pts = 5 + rng.range(0, 5);        // 5..10 起底
while (rng.range(1, 6) == 6) pts += 10;  // 每 1/6 機率再 +10，可疊加
```

預期分佈：

| Bonus | 機率 | 能開的職業 |
|-------|------|----------|
| 5-10 | ~83% | Fighter / Mage / Priest / Thief（標準四班） |
| 15-20 | ~14% | **Bishop**（雙系慢一階）+ 大部分 Samurai/Lord |
| 25-30 | ~2.5% | **Samurai / Lord 穩開**（含 LUC 15） |
| 35+ | <0.5% | **Ninja 神階**（所有屬性 15+） |

**reroll 的時間成本算式**：
- 平均 **30 次 reroll** 才骰得到 **25+ bonus**（25 × 30 秒 / 次 = 15 分鐘）
- 平均 **200 次 reroll** 才骰得到 **35+ bonus**（200 × 30 秒 / 次 = 1.5 小時）

> **老玩家忠告**：**1981 年我們是用 Apple II reset 鈕 reroll，2026 年你按 R 就重骰**。
> 不要為了 Ninja 卡 1.5 小時——**Samurai 起步 + 中期轉職** 才是聰明做法。
> 詳見 [六、八職業專業攻略](#classes) 的轉職時機表。

### 1.4 隊伍合理組成

| 位置 | 排 | 推薦職業 | 為什麼不全戰士 |
|------|----|---------|-------------|
| 1 | 前 | Fighter（**矮人**） | 高 HP 主肉 |
| 2 | 前 | Samurai / Lord（**人類 / 矮人**） | 副攻 + 後期自帶法術 |
| 3 | 前 | Fighter / Ninja（**哈比**） | 高 AGI 先攻 |
| 4 | 後 | Priest（**矮人 / 地侏**） | 治療主軸 |
| 5 | 後 | Mage（**精靈**） | 群攻法術 |
| 6 | 後 | Bishop（**精靈**） | 鑑定 + 雙系法術備援 |

**為什麼不全戰士**：
- B6F 之後**對單目標物理 dps = 滅團**——敵群是 9 隻一波，沒群攻你打不完
- B8F Vampire **每擊吸 1-2 級**，沒 **ZILWAN**（牧師 5 級對不死）你重練 50% 經驗
- B10F Werdna **一回合可以丟 TILTOWAIT 80 點群傷**，沒 **MONTINO**（牧師 2 級靜默）全隊滅

> **「兩肉兩法兩混」** 才是老玩家配置——`docs/WALKTHROUGH.md` 也是這個配置。
> 詳細路線請看 [WALKTHROUGH.md 一、通用知識：推薦小隊配置](WALKTHROUGH.md#general)。

---

<a name="early"></a>
## 二、前期 B1F-B3F — 從 0 到 5 級

> 前期最大的迷思是「**B1F 的怪很弱所以可以亂走**」。
> 錯。**B1F 的迷宮設計是 Wizardry 1 整個遊戲最殘酷的學習曲線**——
> 你會在第一個小時內學到「**儲存點不在迷宮裡**」「**屍體會卡在原地**」「**沒撤退就會滅團**」三件事。
> 但 B1F 也有一個**全系列最有名的 XP 農場**，撐過第一晚就能跳級到 5 級。

### 2.1 Murphy's Ghost（墨菲之鬼）經驗農場 — 老玩家共同記憶

<img src="sprites_4up/PCECD_MS_Lvl1_Mage.png" width="280" alt="Lvl1 Mage sprite — placeholder for Murphy's Ghost" align="right" />

**位置**：B1F `5N-13E` 鑲滿寶石的兜帽人雕像（詳見 [WALKTHROUGH.md B1F](WALKTHROUGH.md#b1f)）。

**為何叫農場**：

| 維度 | 數值 |
|------|------|
| HP | 1d100 = 平均 50（每次浮動，**有時 20，有時 100**） |
| AC | **-3**（1 級小隊難命中） |
| 傷害 | **1d1+1 = 2 點**（**幾乎不傷你**） |
| XP | **4,450**（B1F 雜兵 80 倍） |
| 重生率 | 打死後 **80% 機率再出**（同一格反覆刷） |

**標準農場流程**（每輪 ~2 分鐘）：

1. **第 0 步**：先回神殿/旅館把全隊 HP 補滿、法力補滿
2. 進場 → 走到 `5N-13E` → `SEARCH` 觸發
3. **第 1 回合**：
   - 牧師 A：**PORFIC**（自身 AC -4，新手保命）
   - 牧師 B：**MILWA**（照明 + 偵測暗門）
   - 法師：**KATINO**（70%+ 機率把 Murphy 睡掉）
4. **第 2 回合起**：
   - 睡眠中 Murphy **AC 變 10**（必中）→ 戰士狂砍
   - 1 個戰士每回合 1d8+1 = 5 → 兩個戰士 10 點 → 5-10 回合解決
5. **死亡後**：80% 機率再戰，連刷 5-10 場 → **整夜練到 5 級**

> **為何 4am 重製版「修了」這個 bug 卻沒修**：tk421.net 的解析說明這原本是 1981 Greenberg 的設計疏失
> （`1d100` 寫成了 `1d100+0`，少了難度補正），但 1985 年 Roe Adams 出 PSP 重製時保留了它——
> **因為玩家已經把它當成「Wizardry 1 必經之路」**。本專案 `assets/data/monsters.json` 也忠實保留。

### 2.2 KATINO 在 B1F-B3F 為何能 100% 制霸

`KATINO`（魔法師 1 級，**集團睡眠 1-3 回合**）在 B1F-B3F 是**作弊級存在**——

- B1F 怪物**抗咒等級 = 1**（怪物等級）→ saving throw 幾乎全失敗
- B2F-B3F 怪物等級 2-3 → 仍有 60-70% 中咒
- 睡眠中怪物 **AC = 10**（必中）+ **不能行動** → 戰士兩刀解決

**為什麼 v3.2 沒「修」KATINO**：Greenberg 設計上 KATINO 對睡眠系免疫怪（不死/Demon）會自動失效，
中前期沒這類怪 = **KATINO 一咒清場**。本專案 `combat.cpp::group_status()` 1:1 實作。

### 2.3 何時可以挑戰 B2F

**最低門檻**：全隊 **3 級** + 至少 **2 件鎖子甲 + 大盾**（AC 達到 4 以下）。

- **過早下 B2F**：Creeping Coins 群一輪 9 個小傷害刺死法師（**[陷阱 1](#pitfalls)**）
- **太晚下 B2F**：你在 B1F 磨 Murphy 練到 10 級才下去——**B2F XP 已經不夠你升級了**

### 2.4 早期金幣怎麼花

| 階段 | 金幣優先順位 |
|------|-------------|
| 1-2 級 | **Boltac 全套 Leather Armor + Buckler**（前排 6 個位置都裝） |
| 3 級 | **Chain Mail**（戰士 / Samurai / Lord 換掉皮甲） |
| 4 級 | **Long Sword**（戰士主武器，1d8 起跳） |
| 5 級 | **存錢**（不要急著買 Potion of Dios，留 800 GP 應急） |

> **不要做的事**：**B1F 就買 Plate Mail（750 GP）**——你 1 級身上才 200 GP，
> 想存到 750 還沒升級就會被 Creeping Coins 刷死。
> 詳細道具表參考 [ITEMS.md](ITEMS.md#basic-weapons)。

---

<a name="mid"></a>
## 三、中期 B4F-B6F — 5 到 10 級的鴻溝

> 中期是 Wizardry 1 **難度斷層最明顯的一段**。
> B4F 一進去你會發現「**怪怎麼忽然全部會施法了**」——B3F 那個 Highwayman 偷錢的世界結束了，
> 從這裡開始**敵法師會丟 MAHALITO 4d6 群燒你、敵牧師會丟 MONTINO 把你法力鎖死**。
> 1985 年的台灣玩家在這裡 **平均卡 2 週**——當年沒有網路，全靠《電腦玩家》月刊四月號的攻略才過得去。

### 3.1 軍隊邊界 — 為何 B4F-B5F 難度跳一階

故事上 **B4F 是 Trebor 軍隊剿滅 Werdna 的最後一層**（詳見 [LORE.md](LORE.md)）。
劇情之外的**機制原因**：

- B4F 起怪物開始有 **Lv3 Mage / Lv3 Priest**——主動施法
- B4F 警報觸發後**所有後續遭遇 +1 群組數**
- 寶箱開始**內建陷阱**（Teleporter / Anti-Mage / Stunner）

**對應準備**：
- **CALFO**（牧師 2 級）每進新格鑑定前方陷阱 — 已實作於 `camp.cpp::cast_camp_spell("CALFO")`
- **MONTINO**（牧師 2 級）反制敵法師
- **盜賊 LV5+ INSPECT** 寶箱（5 級才能穩定識破 Anti-Mage 陷阱）

### 3.2 Trebor's Tease — B4F 假電梯機關

B4F 的 **Blue Ribbon（藍緞帶）** 房間：撿了緞帶後可以搭電梯到 B9F——
但**這是陷阱**。1 級隊伍直接跳 B9F = 整隊被 Lifestealer 一輪屠光。

**正確順序**：
1. B4F 拿藍緞帶 → **存起來**
2. B5F-B8F 正常爬到 8-9 級
3. **8 級時回 B4F** 搭電梯跳 B9F → 跳過 B9F 直接 B10F

> 1981 年的玩家自己摸索半年才發現這個陷阱。**1986 年中文攻略本上錯了寫「藍緞帶直接通關」**——
> 害一整代台灣玩家送了好幾組隊伍。

### 3.3 B6F 控制面板 — 開關 B7F 隱藏出口

B6F 有個房間裡有**四個開關**，**全開** 後 B7F 出現新的捷徑通道。
但**多按了一個就會關閉所有 B7F 入口**——`docs/MAPS.md` 的 B7F 地圖有標示。

> **保險作法**：到 B6F 先用 **DUMAPIC**（魔法師 1 級）記座標，**只動兩個開關測試**——
> 不對就回頭再動其他。

### 3.4 5 級咒語解鎖 — 群體治療 + 群體傷害

| 法師 | 牧師 |
|------|------|
| **LAHALITO**（4 級，6d6 巨火，群傷） | **DIAL**（4 級，3d8 治療，**單體大量**） |
| **MADALTO**（5 級，8d8 暴雪，群傷） | **BADIAL**（4 級，4d6 傷害，**對單體**） |
| **MAKANITO**（5 級，10 級以下**秒殺**） | **MAPORFIC**（4 級，**全隊 -2 AC 整場戰鬥**） |

> **MAKANITO 戰術**：B6F 之前**所有怪物等級 10 以下**——MAKANITO 一咒**整群秒殺**。
> 但 B7F 起 Arch Mage（Lv7）、Vampire（Lv13）免疫——之後就要換 **LAKANITO**（6 級真空窒息）。

### 3.5 復活 vs 重練 的成本算

中期角色死亡後：

| 選項 | 成本 | 風險 |
|------|------|------|
| **DI**（神殿 7 級牧師，250 GP × char_level） | 5 級 = 1,250 GP | **失敗 = Ashes**，再失敗 = **Lost 永刪** |
| **KADORTO**（神殿 7 級牧師，500 GP × char_level） | 5 級 = 2,500 GP | 比 DI 成功率高，仍可能 Lost |
| **重練 1 級**（培訓場） | 0 GP | 從 1 級開始 + **0 經驗** |

> **5 級以下死亡建議直接重練**——復活費比裝備還貴，重練 + Murphy 農場一晚就回來。
> **8 級以上才值得復活**（裝備 + 法術記憶不能轉）。

---

<a name="late"></a>
## 四、後期 B7F-B9F — 10 到 13 級的等級流失地獄

> 後期就是 **「等級流失防衛戰」**。
> B7F 的 Arch Mage 一個 TILTOWAIT 全滅你；B8F 的 Vampire 一擊吸你 2 級；
> B9F 的 Lifestealer 一擊吸你 3 級 + 麻痺。
> **這一段你看完會傻眼**——1985 年的台灣玩家在 B9F 平均**滅團 5-8 次**才能爬上去。

### 4.1 Vampire / Vampire Lord — 等級吸取的恐怖

<img src="sprites_4up/PCECD_MS_Vampire.png" width="280" alt="Vampire — PCE-CD theme" align="right" />

本專案 v1.21 實作於 `combat.cpp::Special::Drain`：

```cpp
if (special == Special::Drain && victim.status != Status::Dead) {
    int drain = 1 + rng.range(0, 1);  // 1-2 級
    victim.char_level = std::max(1, victim.char_level - drain);
}
```

**Vampire**（B8F）每擊 -1~-2 級；**Vampire Lord**（B10F）每擊 -2~-3 級。
**沒有抗性檢定，無條件生效**——這就是為什麼 1985 年的攻略本第一條警告就是：

> **「看到吸血鬼就跑——除非你準備好 ZILWAN」**

**ZILWAN 對策**（魔法師 6 級 / Bishop 9 級）：
- 對單個**不死系**：3d8 × 8 = 期望值 **~96 傷害**
- Vampire HP 10d8 ≈ 45 → **一咒秒殺**
- Vampire Lord HP 13d8 ≈ 58 → **一咒大概率秒殺**

詳見 [SPELLS.md ZILWAN 條目](SPELLS.md#mage-6) 和 [MONSTERS.md Vampire](MONSTERS.md#tier3)。

### 4.2 B7F Arch Mage — 自帶 TILTOWAIT 的死神

<img src="sprites_4up/PCECD_MS_Arch_Mage.png" width="280" alt="Arch Mage — PCE-CD theme" align="right" />

| 屬性 | 值 |
|------|---|
| HP | 7d8 ≈ 28 |
| AC | **-2**（前排打不太到） |
| XP | **15,880**（等同 Werdna！） |
| 法術庫 | TILTOWAIT（10d15 = **80 點全隊群傷**） |

**先機決定生死**：
- **回合 0**（突襲判定 `combat.cpp::Surprise`）：贏先發 → 法師立刻 **MONTINO 靜默**
- **輸先發** → Arch Mage 立刻 TILTOWAIT → **80 點 / 6 人 ≈ 13 點每人** → 前排可能還活、後排全死
- **平手** → 看初始 AGI

> 老玩家口訣：「**看到 Arch Mage 不丟 MONTINO 的隊伍，下一秒就是滅團畫面**」。

### 4.3 ZILWAN 對不死系秒殺鍊

不死怪在 B7F-B10F 滿地都是。**ZILWAN 鍊** 的標準分工：

| 施法者 | 法術 | 對象 |
|--------|------|------|
| 主教 / 法師 A | **ZILWAN** | Vampire 群 1 |
| 主教 / 法師 B | **ZILWAN** | Vampire 群 2 |
| 牧師 A | **BADIAL** | Lifestealer（不死但 ZILWAN 算 attack roll，BADIAL 穩 4d6） |
| 牧師 B | **MADI**（**MADI** 全隊滿血） | 全隊回血 |

### 4.4 B9F 滑梯捷徑 — 西邊 0N-1W

B9F 西邊 `0N-1W` 有個 **Chute（滑道）** 直通 **B10F 隨機座標**——這是**通往 Werdna 的最短路徑**。

**前提**：
- **滿血 + 滿 MP**（B10F 起點不一定有友好格）
- **至少 5 張 Potion of Madi**（緊急全隊治療）
- **MALOR 就位**（魔法師 7 級緊急傳送回城）

詳見 [WALKTHROUGH.md B9F](WALKTHROUGH.md#b9f)。

### 4.5 MAPORFIC — 9 級牧師最值得的咒語

`combat.cpp::cast_spell` 對 MAPORFIC 的實作：

```cpp
else if (n == "MAPORFIC") {
    party_ac_buff(s, caster, "MAPORFIC", -2);
}
```

**全隊 AC -2，效果到下次戰鬥**——`combat.h::ActiveBuff::combat_only = false`。
意思是**一次施法管整層**。配合戰士 +2 板甲（AC -3）→ 實質 AC -5 → 高階怪命中率掉 40%。

### 4.6 通關前的裝備檢查清單

進 B10F **前**檢查：

- [ ] 戰士 / Samurai / Lord **每人 +2 武器以上**
- [ ] Samurai / Lord 之一裝備 **Muramasa Blade**（傳奇武器，**10% 即死機率**）
- [ ] 全隊**至少 +1 防具**
- [ ] 至少 1 個 **Ring of Healing**（每回合自動 +1 HP）
- [ ] **5 張以上 Potion of Madi**（全隊滿血藥）
- [ ] **3 張以上 Scroll of MALOR**（緊急回城）
- [ ] **2 張以上 Scroll of Tiltowait**（保底群攻）

> 詳細道具表參考 [ITEMS.md 傳奇武器](ITEMS.md#legendary-weapons)。

---

<a name="endgame"></a>
## 五、終戰 — B10F 與 沃登納（Werdna）

> 1985 年某個深夜，《電腦玩家》編輯部第一次看到玩家投稿 Werdna 通關截圖——
> 黑白螢幕、底片相機、印在雜誌上模糊得看不清字。
> 但那位玩家在投稿信上寫了一行字：
> > **「我花了三個月。」**
>
> 三個月。三個月 reroll、三個月畫地圖、三個月被等級吸取重練。
> 然後在 B10F 的 Werdna 房，他丟了 MONTINO，**Werdna 沒能放 TILTOWAIT**——通關。
>
> 這就是 Wizardry。

<img src="sprites_4up/PCECD_MS_Werdna.png" width="280" alt="Werdna — PCE-CD theme" align="right" />

### 5.1 Contra Dextra Avenue 暗語

B10F 入口石碑寫 **「CONTRA DEXTRA AVENUE」**——拉丁文 + 法文 = **「走右邊的反面」** = **「全部左轉」**。

**標準路徑**：每個 chamber 出口 **永遠選左**。選右會被傳送回起點。**左轉 5 次** = 到 Werdna 房。

詳見 [WALKTHROUGH.md B10F](WALKTHROUGH.md#b10f)。

### 5.2 6 個守衛戰簡介

| Chamber | 守衛 | 主要威脅 |
|---------|------|---------|
| 1 | 4× Master Ninja | 1 擊斬首 5% 機率（**老玩家心臟病兇手**） |
| 2 | 4× Greater Demon | TILTOWAIT × 1（中等） |
| 3 | 4× Vampire Lord | 每擊 **-2~-3 級**（用 ZILWAN） |
| 4 | 4× High Ninja + 2× Fire Giant | 物理 dps 爆炸 |
| 5 | 4× Arch Mage | **每個都會 TILTOWAIT**（MONTINO 全靜默） |
| 6 | 4× Lifestealer + 1× Wisp | 等級吸取 + 麻痺 |

**6 場連戰 + 不能回城** → 必須**事先準備 5+ Potion of Madi**。

### 5.3 沃登納戰術 — 回合 by 回合

**敵人組成**：
- **Werdna**（HP 60、AC -7、有 TILTOWAIT / MAHAMAN / MAKANITO）
- **Vampire Lord** × 1
- **Vampire** × 4

#### 回合 1（**Initiative 必勝戰術**）

| 角色 | 動作 | 理由 |
|------|------|------|
| 法師 | **MONTINO**（目標 Werdna 群） | **這是整場戰鬥最重要的一個動作**——Werdna 被靜默就不能 TILTOWAIT |
| 主教 / 法師 B | **ZILWAN** | 秒殺 1 個 Vampire |
| 牧師 A | **BADIAL**（目標 Vampire Lord） | 4d6 削血 |
| 牧師 B | **MABADI**（目標 Vampire Lord） | 把 VL HP 削到 1d8 |
| 前排戰士 ×3 | **全部砍 Werdna** | HP 60，戰士 +2 一回合 ~ 10 dmg × 3 = 30 |

#### 回合 2（清場）

| 角色 | 動作 | 理由 |
|------|------|------|
| 法師 | **TILTOWAIT**（全敵群） | 10d15 = 80 點 → 清掉所有 Vampire |
| 牧師 A | **MADI**（全隊） | 滿血 |
| 牧師 B | **DIAL** 給最殘戰士 | 補單體 |
| 前排 | **集火 Werdna 剩血** | 30 + 30 = 60，剛好打死 |

### 5.4 為何「絕對不可以」讓 Werdna 先施 TILTOWAIT

`combat.cpp` 對 TILTOWAIT 的實作：

```cpp
else if (n == "TILTOWAIT") all_dmg_spell(s, caster, "TILTOWAIT", 10, 15, "滅世");
```

10d15 = **期望 80 點群傷** / 6 人 = **每人 13 點**。
12 級戰士 HP 約 60 → 還能撐；12 級法師 HP 約 20 → **一咒死光**。
**法師死 = 沒人接 MONTINO/MADI/TILTOWAIT** = 滅團不可逆。

> **這就是 Wizardry 1 唯一一條鐵律**：**法師先發 = 你贏；Werdna 先發 = 你死**。

### 5.5 終戰備援

- **MOGATO**（不存在的咒語，老玩家誤傳——本作沒實作）→ 改用 **PORFIC + MAPORFIC** 雙堆 AC
- **LATUMAPIC**（牧師 3 級）**戰前預先 ID**——避免假名怪藏 Vampire Lord（**[陷阱 5](#pitfalls)**）
- **MOLITO**（魔法師 3 級閃電群傷）作 TILTOWAIT 失效備援

---

<a name="classes"></a>
## 六、八職業專業攻略

> 1981 年的 Wizardry 只有 4 個基本職業 + 4 個菁英——這個設計影響了之後 30 年的 RPG。
> 龍與地下城的多職業設定、Final Fantasy 的轉職系統、甚至 2010 年代的 Diablo 3 都可以追到 Greenberg 的這 8 個格子。
> 本專案 `core/character.h::Klass` 完整保留 1981 列舉，每個職業都有獨立的屬性檢核、HP 公式、法術表。

### Fighter（戰士）

- **屬性**：STR 11+
- **HP 公式**：`base 10 + level × (per_level 1d10 + vit_bonus)` — `rules.cpp::recompute_derived()`
- **轉職時機**：**到 8 級不轉**（戰士 HP 池冠軍）
- **最佳裝備**：Plate Mail + Tower Shield + Long Sword +2
- **戰鬥定位**：前排，主肉

### Mage（魔法師）

- **屬性**：IQ 11+
- **HP 公式**：base 4 + level × (1d4 + vit_bonus)
- **轉職時機**：學會 **TILTOWAIT** 後（13 級）才考慮，**不要太早轉**
- **最佳裝備**：Dagger of Speed + Robe + Wizard Hat
- **戰鬥定位**：後排，群攻

### Priest（牧師）

- **屬性**：PIE 11+
- **HP 公式**：base 8 + level × (1d8 + vit_bonus)
- **轉職時機**：到 9 級學會 **MADI** + **MAPORFIC** 後可以轉
- **最佳裝備**：Mace +2 + Chain Mail + Holy Symbol
- **戰鬥定位**：後排，治療 + buff

### Thief（盜賊）

- **屬性**：AGI 11+
- **HP 公式**：base 6 + level × (1d6 + vit_bonus)
- **轉職時機**：到 10 級（INSPECT 寶箱成功率 95%）後轉 **Ninja**
- **最佳裝備**：Short Sword + Leather Armor +2
- **戰鬥定位**：前排 3 號位，先攻 + 寶箱

### Bishop（主教）

- **屬性**：IQ 12+ PIE 12+ 非 Neutral
- **HP 公式**：base 5 + level × (1d6 + vit_bonus)
- **特性**：**雙系法術，但牧師延遲 3 級**（`rules.cpp::fill_slots(.., 3)`）
- **轉職時機**：到 13 級雙系全滿
- **戰鬥定位**：後排，鑑定 + 雙系法術備援

### Samurai（武士）

- **屬性**：STR 15 IQ 11 PIE 10 VIT 14 AGI 10 非邪惡
- **HP 公式**：base 12 + level × (1d10 + vit_bonus)
- **特性**：**戰士能力 + 魔法師法術（延遲 3 級）**
- **最佳裝備**：**Muramasa Blade**（10% 即死）+ Plate Mail
- **戰鬥定位**：前排，**全遊戲最強職業之一**

### Lord（領主）

- **屬性**：STR 15 IQ 12 PIE 12 VIT 15 AGI 14 LUC 15 善良
- **HP 公式**：base 12 + level × (1d10 + vit_bonus)
- **特性**：**戰士能力 + 牧師法術（延遲 3 級）**
- **最佳裝備**：Holy Sword + Plate Mail of Lords
- **戰鬥定位**：前排，**自帶治療**

### Ninja（忍者）

- **屬性**：所有屬性 15+ 必須邪惡（**本專案 v3.2 修正**：原版 17+，v3.2 改 15+）
- **HP 公式**：base 8 + level × (1d8 + vit_bonus)
- **特性**：**裸體 AC 隨等級下降**（13 級 AC = -3 不穿裝甲）+ **5% 斬首**
- **戰鬥定位**：前排 3 號位，先攻

### v3.2 規則修正影響

本專案 `src/core/rules.cpp::class_requirements_met()` 套用 v3.2 修正：

| 修正項 | 1981 原版 | v3.2 / 本專案 |
|--------|-----------|--------------|
| Ninja 最低 | **17+** 所有屬性 | **15+** 所有屬性 |
| Bishop 牧師線 | 立即 | **延遲 3 級** |
| Samurai 法師線 | 立即 | **延遲 3 級** |
| Lord 牧師線 | 立即 | **延遲 3 級** |

> v3.2 是 Greenberg 在 1984 年釋出的官方修正——本專案優先採用，這在 [MANUAL_GAP.md](MANUAL_GAP.md) 有完整列表。

---

<a name="spells"></a>
## 七、51 咒語優先級表

> Wizardry 51 個咒語，**至少 30 個 1985 年沒人用**——
> 不是因為設計爛，是因為**遊戲節奏不需要**。
> 真正能改寫戰局的就那 12 個——這是 45 年社群共識。

### 7.1 必修 12 咒（從低到高）

| 咒語 | 級數 | 學派 | 為什麼必修 |
|------|------|------|----------|
| **KATINO** | M1 | 魔法 | B1F-B3F 制霸 + Murphy 必備 |
| **DUMAPIC** | M1 | 魔法 | 座標定位（沒這個 Spinner 直接迷路） |
| **MILWA** | P1 | 牧師 | 照明 + 偵測暗門（B3F 起必開） |
| **PORFIC** | M1 | 魔法 | 緊急 AC -4 自身（Murphy 戰必用） |
| **MOGREF** | M1 | 魔法 | AC -2 自身備援 |
| **MAHALITO** | M3 | 魔法 | 4d6 群傷，B4F-B6F 主力 |
| **DIAL** | P4 | 牧師 | 3d8 治療單體（戰鬥救命） |
| **LATUMAPIC** | P3 | 牧師 | **看穿假名怪**（沒這個踩坑 5） |
| **MAPORFIC** | P4 | 牧師 | **全隊 AC -2 整層**（中後期王道） |
| **ZILWAN** | M6 | 魔法 | 對不死秒殺（Vampire 剋星） |
| **MONTINO** | P2 | 牧師 | **靜默敵法師**（Arch Mage / Werdna 剋星） |
| **TILTOWAIT** | M7 | 魔法 | 10d15 滅世（終戰清場） |

詳細效果與戰術說明參考 [SPELLS.md](SPELLS.md)。

### 7.2 可跳過的 5 咒

| 咒語 | 級數 | 為何跳過 |
|------|------|---------|
| **BADIOS** | P1 | 1d8 對單體傷害——**牧師打人**不如戰士物攻 |
| **LOKTOFEIT** | P6 | 隨機傳送 + 金幣全失 + 道具全失——**v3.2 已修但仍邊緣** |
| **HAMAN** | M7 | 隨機效果太爛（**有時還幫敵人**） |
| **MALOR** | M7 | 緊急回城用 Scroll 就好，不值得記憶 |
| **MAHAMAN** | M7 | 同上隨機，**回卷軸帶就好** |

### 7.3 為何 LAKANITO 是 6 級最值得

`combat.cpp::cast_spell("LAKANITO")` 實作：

```cpp
else if (n == "LAKANITO") {
    // 10% × HD% chance, ignores level filter (true vacuum suffocation)
    instant_kill_group(s, caster, a, "LAKANITO", 99);
}
```

**10% × HD% 機率秒殺整群**——B7F+ 一群 Lv7 怪 → 10% × 7 = **70% 整群秒**。
比 MAKANITO（10 級以下，B7F 後失效）泛用。

---

<a name="pitfalls"></a>
## 八、五大「不要踩」陷阱

> 老玩家死過的坑，你不需要再死一次。

### 陷阱 1：1 級進 B2F = Creeping Coins 群刺死

**症狀**：1 級隊伍剛進 B2F → 一房間 9 隻 Creeping Coins → 每隻 2 HP 但有 9 隻 → 9 個 1d4 攻擊一輪打你法師 → 法師死。

**對應**：**至少 3 級 + 全隊 Leather Armor 才下 B2F**。

### 陷阱 2：Vampire 不撤退

**症狀**：B8F 遇到 Vampire 群 → 第 1 回合 Vampire 行動 → 戰士被吸 2 級 → **整隊崩盤**（HP 上限掉 / 法術忘掉）。

**對應**：
- **看到 Vampire 群第一動作 = `RUN`**（如果沒 ZILWAN）
- **有 ZILWAN = 第 1 回合 ZILWAN 秒殺**（不要先砍）

### 陷阱 3：Werdna 戰不 MONTINO

**症狀**：法師沒記 MONTINO → Werdna 回合 1 丟 TILTOWAIT → 80 點群傷 → 法師死 → 第 2 回合無法 MADI 群補 → 滅團。

**對應**：**進 B10F 前**確認法師 / 主教**法術庫第 2 級欄位有 MONTINO**。

### 陷阱 4：全 Good 隊伍 — B6F 之後對 Evil 角色有 XP bonus

**症狀**：全 Good 隊伍 → B6F 之後**所有 Good 怪物**（守衛士兵）給的 XP 變少 → 升級速度落後 → B9F 等級不夠被屠。

**對應**：**至少帶 1 個 Evil 角色**（Ninja 或 Evil Samurai）。或全 Neutral——Neutral 對所有 alignment 都不變動。

> 本專案 `combat.cpp` 對 alignment XP 調整為「按怪物 alignment 與隊伍主流不同 +10%」。
> 詳見 [MANUAL_GAP.md](MANUAL_GAP.md) v1.x 變動。

### 陷阱 5：不用 LATUMAPIC — 假名怪藏 Vampire Lord

**症狀**：戰鬥開始顯示「***WIZARD***」（未鑑定名）→ 你以為是 Lv5 Mage → 派戰士衝→ 結果是 Vampire Lord → 戰士被吸 3 級。

**對應**：
- **每次戰鬥開頭** 法師 / 牧師之一**先施 LATUMAPIC**
- `combat.cpp` 對 LATUMAPIC 實作：`g.identified = true` → 顯示真名

> 本專案 v1.22 把 LATUMAPIC 接到 `display_name()`——未鑑定名就是 1981 原版的 *CREATURE* / *DOG-LIKE* / *WIZARD*。
> 詳見 [MONSTERS.md](MONSTERS.md) 每隻怪的 *未鑑定名* 欄位。

---

<a name="implementation"></a>
## 九、本專案實作差異速查（v0.x-v1.24）

> 本專案不是「1981 原版的完美 1:1 復刻」——它是「**為 2026 年玩家優化過的 v3.2 規則基準**」。
> 這節列出**跟原版不同的地方**，老玩家請特別注意。

### 9.1 v3.2 修正在本專案已生效

- ✅ Ninja 最低 15 而非 17（`rules.cpp::class_requirements_met`）
- ✅ Bishop / Samurai / Lord 雙系延遲 3 級（`rules.cpp::fill_slots(..., 3)`）
- ✅ MONTINO 不再 bug 失效（`combat.cpp::group_status`）
- ✅ Poisoned 狀態每輪扣血（`combat.cpp::resolve_round` 末尾）
- ✅ 前/後排陣型（後排 slots 3-5 melee 失敗）

### 9.2 「為現代玩家」的 QoL 調整

| 項目 | 1981 原版 | 本專案 |
|------|----------|--------|
| 存檔槽 | 1 槽 | **5 槽**（`gamesave.cpp`） |
| 法力補滿 | 旅館投宿才補 | **旅館自動補滿**（`inn.cpp:233 restore_spell_slots`） |
| 主題切換 | 無 | **F3 切換 PCE-CD / Mono / Outline / Sepia / PC98 / WSC / Mac**（`theme.cpp`） |
| 語言切換 | 英文 | **F4 切繁中 / English / 日本語**（`tr.cpp::cycle_lang`） |
| 永久死亡屍體 | 留在原地 | **可回收 + 神殿復活**（`traps.cpp::check_body_pickup`） |
| 法術自動學習 | 升等選學 | **自動學完該級全咒**（`rules.cpp::recompute_spell_slots`） |
| 暗黑區 MILWA | 無視 | **MILWA / LOMILWA 步數遞減**（`maze.h::dark_zone` + `state.light_steps_left`） |
| 結局畫面 | 無（回城就結束） | **Scene::Ending 含 splash**（`assets/themes/pcecd/ending/background.png`） |

### 9.3 還沒實作（v1.24 仍 ⚠）

- ⚠ Werdna's Amulet **不是真 inventory 道具**（劇情上由 ending 場景代為持有）
- ⚠ **NG+ 新遊戲 +** 沒做
- ⚠ Charge / break 計次器（法杖類有限次施法）
- ⚠ alignment-locked 裝備（邪派不能裝聖物）
- ⚠ 70+ 歲自然死亡檢定

完整對照請看 [MANUAL_GAP.md](MANUAL_GAP.md)。

---

<a name="sources"></a>
## 十、引用來源

### 攻略本與社群文獻
- [tk421.net — Wizardry 1 Walkthrough](https://www.tk421.net/wizardry/wiz1walk.shtml) — 1996 年立基的英文社群聖經
- [GameFAQs — Kelly R. Flewin Wizardry 1 Guide](https://gamefaqs.gamespot.com/pc/564837-wizardry-proving-grounds-of-the-mad-overlord/faqs/7432)
- [Wizardry Wiki (wiki.gg)](https://wizardry.wiki.gg/) — 怪物 stat block + 法術 catalogue
- [Wizardry Fandom Wiki](https://wizardry.fandom.com/) — Werdna 戰術 + Contra Dextra Avenue 解讀
- [The-Spoiler — Wizardry 1 Walkthrough](https://the-spoiler.com/RPG/Sir-Tech/wizardry.1.2.html)
- [CRPG Adventures — Wizardry Level 10](http://crpgadventures.blogspot.com/2021/09/wizardry-level-10.html)
- Sir-Tech 1981 原版手冊（**Ultimate Wizardry Archives** 1998 Interplay 再版，p18, p33–36, p52+）

### 本專案 cross-reference
- [WALKTHROUGH.md](WALKTHROUGH.md) — 10 層完整路線
- [MONSTERS.md](MONSTERS.md) — 30 隻怪物 stat + 4-up sprite
- [SPELLS.md](SPELLS.md) — 51 咒語語法 + 戰術
- [ITEMS.md](ITEMS.md) — 30 道具 + 詛咒警告
- [MAPS.md](MAPS.md) — 10 層 SVG 地圖
- [LORE.md](LORE.md) — Trebor / Werdna / Boltac 世界觀
- [MANUAL_GAP.md](MANUAL_GAP.md) — 實作對照表

### Codebase 直接引用
- `src/core/character.h` — Klass / Race / Alignment 列舉
- `src/core/rules.cpp` — base_attr / roll_bonus_points / fill_slots / recompute_spell_slots
- `src/core/combat.cpp` — Special::Drain / Paralyze / Breath / Surprise / cast_spell
- `src/game/roller.cpp` — 屬性骰點 + 職業檢核 UI
- `src/game/inn.cpp` — TempleUI::Uncurse / restore_spell_slots
- `src/game/traps.cpp` — 8 種陷阱 + switch_floor

---

> **致 1985 年那位投稿 Werdna 通關截圖的玩家**：
> 你的三個月沒有白費。
> 41 年後的我們，把你那張黑白底片變成了 16:9 全彩重製版——
> 但 Murphy's Ghost 還在 `5N-13E`，KATINO 還是 100% 制霸 B1F，
> Werdna 還是會在你不丟 MONTINO 的回合丟 TILTOWAIT。
>
> 這就是 Wizardry——故事沒有真正結束。
>
> *（攻略內容為自行撰寫之機制摘要。原始作品版權屬 Sir-Tech / IPLAY。）*
