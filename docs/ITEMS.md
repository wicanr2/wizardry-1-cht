# 道具圖鑑 — 武器、防具、藥水、神器全集

> **為什麼讀這篇？**
> Wizardry I 的道具系統有兩個讓 1981 年玩家崩潰的設計：
> **第一**：所有道具進入背包時**都是「未鑑定」狀態**——你看到的是「**? STRANGE OBJECT**」，
> 必須付錢給 Boltac 鑑定（或讓主教吃 MP 鑑定）才能知道是什麼。
> **第二**：**詛咒道具**鑑定後**已經來不及**了——只要你在戰鬥外把它裝上去，
> **它就脫不下來**，除非付 1,000,000 金幣給 Boltac 解除（**1981 年 1 隻 Vampire 給 29,400 XP**——
> 自己換算 100 萬金幣有多狠）。
>
> 這篇是把 `items.json` 30 個道具配上**價格、傷害、職業限制、特殊效果、詛咒警告**，
> 再加上 W1 完整道具庫的擴充說明（**Boltac 賣不到的稀有戰利品**）。

---

## 目錄

1. [道具系統機制](#mechanics)
2. [基礎武器（B1F–B3F 可購買）](#basic-weapons)
3. [魔法武器（戰利品）](#magic-weapons)
4. [傳奇武器（B7F+ 寶箱）](#legendary-weapons)
5. [防具系列](#armor)
6. [盾與頭盔](#shield-helm)
7. [藥水與卷軸](#potions-scrolls)
8. [戒指與護符](#rings-amulets)
9. [詛咒道具警告](#cursed)
10. [Werdna's Amulet — 終極獎勵](#werdna-amulet)
11. [Boltac 鑑定價格表](#boltac)
12. [引用來源](#sources)

---

<a name="mechanics"></a>
## 一、道具系統機制

### 未鑑定狀態

掉落的道具進入背包時是「**未鑑定**」——顯示為：
- `? STRANGE OBJECT`（神秘物品）
- `? RING`、`? AMULET`、`? POTION`
- `? SWORD`、`? STAFF`

**裝備未鑑定道具 = 賭運氣**——可能是 +2 武器，也可能是詛咒物。

### 鑑定方法

| 方法 | 成本 |
|---|---|
| **Boltac 鑑定** | 物品價格 × 50% |
| **主教 INSPECT** | 1 MP（**主教等級越高失敗率越低**） |
| **CALFO 咒語** | 牧師 2 級，只能鑑定**寶箱陷阱**（不能鑑定道具） |

> **省錢秘訣**：B1F 雜兵掉落便宜道具，**找主教鑑定**比較划算；
> B8F+ 高價道具讓 **Boltac 鑑定**比較穩。

### 職業限制（Use 欄位）

`items.json` 的 `use` 欄位用字母縮寫：

| 字母 | 職業 | 中譯 |
|---|---|---|
| **F** | Fighter | 戰士 |
| **M** | Mage | 魔法師 |
| **P** | Priest | 牧師 |
| **T** | Thief | 盜賊 |
| **B** | Bishop | 主教 |
| **S** | Samurai | 武士 |
| **L** | Lord | 領主 |
| **N** | Ninja | 忍者 |

例：`"use":["F","S","L","B"]` = 戰士 / 武士 / 領主 / 主教**可裝備**。

---

<a name="basic-weapons"></a>
## 二、基礎武器（B1F–B3F 可購買）

### 匕首 (Dagger, ID 5)

| 屬性 | 值 |
|---|---|
| 中譯 | 匕首 |
| 價格 | 5 GP |
| 傷害 | 1d4 |
| 揮擊次數 | 1 |
| 適用 | **全 8 職業**（F/M/P/T/B/S/L/N） |

**用途**：1 級魔法師唯一買得起的武器。**便宜、輕、所有人都能拿**。

### 法杖 (Staff, ID 4)

| 價格 | 5 GP | 傷害 | 1d4 | 適用 | F/M/P/B |
|---|---|---|---|---|---|

**用途**：法師、牧師起手武器。**可惜傷害低**。

### 短劍 (Short Sword, ID 2)

| 價格 | 15 GP | 傷害 | 1d6 | 適用 | F/S/L/T/N/B |
|---|---|---|---|---|---|

**用途**：盜賊、忍者前期主力。**比匕首傷害高 50%**。

### 長劍 (Long Sword, ID 1)

| 價格 | 25 GP | 傷害 | 1d8 | 適用 | F/S/L/B |
|---|---|---|---|---|---|

**用途**：戰士標配。**期望值 4.5/擊**。

### 釘頭錘 (Mace, ID 3)

| 價格 | 30 GP | 傷害 | 1d6 | 適用 | F/P/B |
|---|---|---|---|---|---|

**用途**：**牧師唯一的非詛咒攻擊武器**（牧師不能用劍）。

---

<a name="magic-weapons"></a>
## 三、魔法武器（戰利品 / 高價購買）

### 長劍+1 (Long Sword +1, ID 14)

| 價格 | 1,500 GP | 傷害 | 1d8+1 | 適用 | F/S/L/B |
|---|---|---|---|---|---|

**戰術**：B3F+ 寶箱可掉。**Boltac 售價 1500**——B1 練到 5 級可以買得起。

### 釘頭錘+1 (Mace +1, ID 15)

| 價格 | 1,500 GP | 傷害 | 1d6+1 | 適用 | F/P/B |
|---|---|---|---|---|---|

### Staff of Mogref (莫格雷夫法杖, ID 16)

| 價格 | 3,500 GP | 傷害 | 1d4 | 特殊 | **25% 機率施 MOGREF**（AC -2） | 適用 | M/B |
|---|---|---|---|---|---|---|---|

**戰術**：**魔法師專屬升級**——同時當武器和 AC buff。

---

<a name="legendary-weapons"></a>
## 四、傳奇武器（B7F+ 寶箱）

### 庫西納特之刃 (Blade of Cusinart, ID 22)

| 屬性 | 值 |
|---|---|
| 中譯 | 庫西納特之刃 |
| 價格 | 15,000 GP |
| 傷害 | **10d10**（**期望 55，最大 100**） |
| 揮擊次數 | 1 |
| 適用 | F/S/L |

**戰術**：B7F+ 寶箱稀有掉落。**戰士配上後，TILTOWAIT 都比不上一刀**。
**彩蛋**：「Cusinart」是 1970 年代美國知名**食物調理機品牌 Cuisinart 的故意拼錯**——
Greenberg 在暗示「**這把劍像料理機一樣把怪物切碎**」。

### 村正之刃 (Muramasa Blade, ID 23)

| 屬性 | 值 |
|---|---|
| 中譯 | 村正之刃 |
| 價格 | **1,000,000 GP**（**全遊戲最貴**） |
| 傷害 | **10d10** |
| 揮擊次數 | **2**（**每回合砍 2 刀**） |
| 適用 | **僅武士 (S)** |
| 特殊 | **50% 機率 +1 力量**（永久） |

**戰術**：**Wizardry I 最強武器**——只有武士能用，**B9F+ 極稀有掉落**。
**期望傷害 110/回合**——**Werdna 一回合解決**。

**彩蛋**：「**村正**」是日本歷史上**最有名的妖刀傳說**——
妖刀殺氣重，據說會吸主人靈魂。**Wizardry 的日本市場成功**讓這把刀後來在
**Final Fantasy / Castlevania / Soul Calibur** 都出現過。

### 手裏劍 (Shuriken, ID 24)

| 屬性 | 值 |
|---|---|
| 中譯 | 手裏劍 |
| 價格 | **1,000,000 GP**（與村正同價） |
| 傷害 | **8d8**（期望 36，最大 64） |
| 揮擊次數 | **3**（**每回合砍 3 次！**） |
| 適用 | **僅忍者 (N)** |

**戰術**：**忍者專屬**——**3 × 8d8 = 期望 108/回合**。
配合忍者的 **5% 斬首** = **Werdna 一回合多刀斬首機率高**。

---

<a name="armor"></a>
## 五、防具系列

防具的 `ac_mod` 是**負數**（**降低 AC = 更難被打到**）。

| ID | 道具 | 中譯 | 價格 | AC | 適用 |
|---|---|---|---|---|---|
| 6 | LEATHER ARMOR | 皮甲 | 50 | -1 | F/P/T/B/S/L/N |
| 7 | CHAIN MAIL | 鎖子甲 | 90 | -2 | F/P/S/L/N |
| 9 | BREAST PLATE | 胸甲 | 200 | -2 | F/P/S/L/N |
| 8 | PLATE MAIL | 板甲 | 750 | **-3** | F/S/L |

**注意**：**法師 (M) 只能穿衣服**——**任何防具都不能穿**——這是 Wizardry 規則（同 D&D）。
**忍者裝甲會抵消「裸體 AC bonus」**——**村正忍者不該穿任何防具**！

### 高階防具（B7F+ 寶箱）

| 道具 | AC | 來源 |
|---|---|---|
| Plate Mail +2 | -7 | B7F+ 掉落 |
| Breast Plate +3 | -7 | B9F+ 寶箱 |
| Cold Chain Mail | -6 + 抗火 | 神殿任務 |
| Garb of Lords | **-10**（**全遊戲最強**） + 自動 +1 HP/回合 | **僅領主 (L)**，B10F 寶箱 |

---

<a name="shield-helm"></a>
## 六、盾與頭盔

### 小盾 (Small Shield, ID 10)

| 價格 | 20 GP | AC | -1 | 適用 | F/P/T/B/S/L/N |
|---|---|---|---|---|---|

### 大盾 (Large Shield, ID 11)

| 價格 | 40 GP | AC | -2 | 適用 | F/P/S/L |
|---|---|---|---|---|---|

### 頭盔 (Helm, ID 12)

| 價格 | 100 GP | AC | -1 | 適用 | F/P/S/L/N |
|---|---|---|---|---|---|

### 鎧甲護手 (Gauntlets, ID 13)

| 價格 | 6,000 GP | AC | -2 | 適用 | F/S/L/N |
|---|---|---|---|---|---|

**戰術**：**6000 GP 換 -2 AC**——B4F+ 才買得起。**戰士組合可達 AC -10**。

---

<a name="potions-scrolls"></a>
## 七、藥水與卷軸

### 治療藥水 (Potion of Dios, ID 18)

| 價格 | 500 GP | 效果 | 戰鬥內外都可施 DIOS（1d8 HP） |
|---|---|---|---|

**戰術**：**任何角色都可喝**——包括戰士。但 500 GP 一瓶，**牧師施 DIOS 是 0 GP**。

### 解毒藥水 (Potion of Latumofis, ID 19)

| 價格 | 250 GP | 效果 | 解毒 |
|---|---|---|---|

**戰術**：對抗 Gas Cloud / Gas Dragon。牧師等級不夠時的緊急備案。

### 卡提諾卷軸 (Scroll of Katino, ID 20)

| 價格 | 500 GP | 效果 | 施 KATINO（敵群睡眠） |
|---|---|---|---|

**戰術**：戰士也可讀卷軸——**1 級戰士拿卷軸也可以睡 Murphy's Ghost**。

### 巴迪奧斯卷軸 (Scroll of Badios, ID 21)

| 價格 | 500 GP | 效果 | 施 BADIOS（1d8 傷害） |
|---|---|---|---|

---

<a name="rings-amulets"></a>
## 八、戒指與護符

### 行動戒指 (Ring of Movement, ID 25)

| 價格 | 5,000 GP | 效果 | 施 DIALKO（解麻痺/睡眠） |
|---|---|---|---|

**戰術**：對抗 Giant Toad / Lifestealer 麻痺。

### 治癒戒指 (Ring of Healing, ID 26)

| 價格 | **300,000 GP** | 效果 | **每回合自動 +1 HP** |
|---|---|---|---|

**戰術**：**全角色適用** + **被動效果**——**走迷宮自動補血**。
**B7F+ 寶箱稀有掉落**——買不起的話祈禱掉落。

### 馬卡尼托護符 (Amulet of Makanito, ID 27)

| 價格 | 4,000 GP | 效果 | 5% 機率施 MAKANITO（殺 8 HD 以下） |
|---|---|---|---|

**戰術**：**戰士也能放窒息咒**——對 B6F–B7F 群怪有效。

### 鎧甲護手（解釋）

`items.json` 還有些**未實裝但歷史上存在**的稀有護符：

| 道具 | 效果 |
|---|---|
| Amulet of Jewels | 0% 施 DUMAPIC（**Snafaru 註：bug 機率太低**） |
| Amulet of Manifo | 10% 施 MANIFO（麻痺敵群） |
| Rod of Flame | 10% 施 MAHALITO + 抗火 |
| Priest's Ring | 效果未知（W1 bug item） |

---

<a name="cursed"></a>
## 九、詛咒道具警告 ⚠️

**詛咒道具**鑑定後**已經晚了**——只要戰鬥外裝備，**無法卸下**，**並有負面效果**：

| 詛咒物 | 偽裝成 | 效果 |
|---|---|---|
| Long Sword -1 | Long Sword +1 | -1 命中、-1 傷害 |
| Short Sword -2 | Short Sword +2 | -2 命中、-2 傷害 |
| Mace -2 | Mace +2 | -2 命中、-2 傷害 |
| Staff -2 | Staff +2 | -2 命中、-2 傷害 |
| Shield -2 | Shield +2 | **AC +2**（**變更糟**） |
| **Ring of Death** | Ring of Healing | **每回合 -3 HP** |

### 卸下方法

| 方法 | 成本 |
|---|---|
| **Boltac 解咒** | **道具價格 × 100%** |
| **DI 復活後重新裝備** | 死亡 + 復活的全部成本（不划算） |
| **裝備到死亡** | 屍體上的詛咒物可被神殿移除 |

> **省錢秘訣**：**陌生道具**進背包後**先給主教 INSPECT 鑑定**，
> **絕不要**在戰鬥外直接裝備測試。

---

<a name="werdna-amulet"></a>
## 十、Werdna's Amulet — 終極獎勵

### 規格

| 屬性 | 值 |
|---|---|
| ID | 17 |
| 名稱 | WERDNA'S AMULET |
| 中譯 | 沃登納護身符 |
| 類別 | misc（特殊） |
| 價格 | 999,999 GP（**Boltac 不收**） |
| 取得 | **打死 Werdna 後 100% 掉落** |
| 是否詛咒 | **是** |

### 效果

- **裝備時：AC -20**（**全遊戲最強防具**）
- **裝備時：每回合 +5 HP**（**全遊戲最強回血**）
- **詛咒：脫不下來**

### 為什麼**千萬不要裝備**

知名攻略作者 Kelly R. Flewin 在他的攻略裡明確警告：

> *「Just DON'T USE IT!」*

原因：

1. **AC -20 + 5 HP/回合聽起來無敵**——確實是。
2. **但戴上就鎖死**——Boltac 收 **1,000,000 GP 解咒**。
3. **無法傳遞** = **不能給其他隊員共享治療效果**。
4. **W2 / W3 / W4 載入此存檔時** = **角色已被永久標記**。

### 正確用法

| 行為 | 為何 |
|---|---|
| **放背包不裝備** | OK |
| **戰鬥前 → 隊員 A 戴上** | 開戰時 AC -20 + 5 HP/回合 |
| **戰鬥後 → 立刻卸下傳給 B** | **A 還沒詛咒鎖死之前** |
| **下次戰鬥 → B 戴上** | 輪流共享 |

**注意**：**裝備滿 1 整個戰役（rest cycle）後鎖死**——精確時長隨版本不同。

---

<a name="boltac"></a>
## 十一、Boltac 鑑定價格表

Boltac 提供四種服務，價格固定為**物品本體價格的百分比**：

| 服務 | 價格 | 備註 |
|---|---|---|
| **鑑定** (Identify) | 50% | 比主教 INSPECT 穩 |
| **解咒** (Uncurse) | 100% | **避免一輩子戴詛咒物** |
| **修復** (Repair) | 50% | 戰鬥中可能損壞 |
| **回收** (Sell back) | 50% | 半價回收 |

### Boltac 的彩蛋

`items.json` ID 29 有個**特殊道具 BOLTAC'S NOTE（波塔克的字條）**——
這是 1981 原版**未實裝**的彩蛋道具，據說是 Boltac 寫給玩家的私人信件，
**只在某些版本的存檔資料庫中存在**，但**從未實裝在遊戲中**。

---

<a name="sources"></a>
## 十二、引用來源

- 本專案 `assets/data/items.json`（30 道具中文名與基底數據）
- [Snafaru — Wizardry 1 Items List](https://www.zimlab.com/wizardry/walk/wizardry-1-items-list.htm)
- [Snafaru — Wizardry 1 Items（recovered）](https://www.zimlab.com/wizardry/recovered/jh/wizardry/items.html)
- [tk421.net — Wizardry 1 Items](https://www.tk421.net/wizardry/wiz1items.shtml)
- [StrategyWiki — Wizardry 1 Items](https://strategywiki.org/wiki/Wizardry:_Proving_Grounds_of_the_Mad_Overlord/Items)
- [JeffLudwig — Wizardry SNES Rebalancing Mod Item Lists](http://jeffludwig.com/wizardry1-2-3/itemlist.php)
- [Wizardry Wiki — Muramasa Blade](https://wizardry.wiki.gg/wiki/Muramasa)
- [The-Spoiler — Wizardry I Solution](https://the-spoiler.com/RPG/Sir-Tech/wizardry.1.1/WIZ1.HTM)
- Sir-tech《Ultimate Wizardry Archives》Manual, 1998 Interplay 再版, p12, p28–32
