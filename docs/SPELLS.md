---
layout: default
title: 巫術咒典 — 51 個咒語完整解析
---

# 巫術咒典 — 51 個咒語完整解析

> **為什麼讀這篇？**
> Wizardry 的咒語不是「火球術」「治療術」這種白話翻譯——
> 它是 Andrew Greenberg 在 1980 年自創的**虛構語言**。
> *HALITO*、*MAHALITO*、*LAHALITO* 三個咒語都是火，但**前綴 MA-/LA- 表示強度遞增**；
> *DIOS* / *BADIOS* 是治療與傷害的**鏡像對**；*TILTOWAIT* 字面是「等等！」（**結果是滅世**）。
> 這套咒語語法**比英文還規律**——你學會 7 個前綴 + 8 個詞根，就能**自己猜出 51 個咒語的意思**。
>
> 這篇是把 1981 年那本沒中文版的手冊 p52+ 重新拆解，
> 用「**詞源 + 公式 + 戰術 + 經典時刻**」四個維度逐條註解。

---

## 目錄

1. [語法總論：前綴與詞根](#grammar)
2. [魔法師咒語（Mage Spells, 21 個）](#mage)
   - [Level 1](#mage-1) / [Level 2](#mage-2) / [Level 3](#mage-3) / [Level 4](#mage-4) /
     [Level 5](#mage-5) / [Level 6](#mage-6) / [Level 7](#mage-7)
3. [牧師咒語（Priest Spells, 30 個）](#priest)
   - [Level 1](#priest-1) / [Level 2](#priest-2) / [Level 3](#priest-3) / [Level 4](#priest-4) /
     [Level 5](#priest-5) / [Level 6](#priest-6) / [Level 7](#priest-7)
4. [v3.2 修正過的咒語](#v32-fix)
5. [實戰咒語組合](#combos)
6. [引用來源](#sources)

---

<a name="grammar"></a>
## 一、語法總論：前綴與詞根

Greenberg 沒有公開咒語語法表，但社群 45 年來歸納出以下規律：

### 前綴（Prefix）

| 前綴 | 意義 | 範例 |
|------|------|------|
| **HA-** | 火、爆裂 | HALITO（小火）、MAHALITO（大火）、LAHALITO（巨火） |
| **MA-** | 加強、群體、巨大 | MAHALITO、MADALTO、MAKANITO、MABADI、MAPORFIC |
| **LA-** | 至高、極致 | LAHALITO、LAKANITO、LATUMAPIC、LATUMOFIS、LOKTOFEIT |
| **DA-** | 冷、霜、冰 | DALTO、MADALTO |
| **DI-** | 治療、生命（牧師） | DIOS、DIAL、DIALMA、DIALKO、DI、KADORTO |
| **BA-** | 傷害、毒、死亡（牧師） | BADIOS、BADIAL、BADIALMA、BADI、BAMATU |
| **KA-** | 消滅、終結、空氣 | KATINO、MAKANITO、LAKANITO、KADORTO、KANDI |
| **MO-** | 隱蔽、移動、變形 | MOGREF、MOLITO、MORLIS、MALOR |
| **TIL-** | 時間、終結 | **TILTOWAIT** |
| **PO-** | 守護、屏障 | PORFIC、MAPORFIC |
| **MI-** | 光、揭露 | MILWA、LOMILWA |

### 詞根（Root）

| 詞根 | 意義 | 範例 |
|------|------|------|
| **-LITO** | 火、發光 | HALITO、MAHALITO、MOLITO、LITOKAN |
| **-ALTO** | 冰、霜 | DALTO、MADALTO |
| **-KANITO** | 消滅、窒息 | MAKANITO、LAKANITO、LITOKAN |
| **-MAPIC** | 看穿、揭示 | DUMAPIC、LATUMAPIC |
| **-OFIS** | 解除、淨化 | LATUMOFIS |
| **-IOS** | 治療、傷害（核心 -IO-） | DIOS、BADIOS |
| **-IAL/ALMA** | 加強的治療/傷害 | DIAL、DIALMA、BADIAL、BADIALMA |
| **-MAN** | 召喚、祈求 | HAMAN、MAHAMAN |
| **-WA** | 光、發散 | MILWA、LOMILWA、TILTOWAIT |
| **-TINO** | 睡眠、沉默 | KATINO、MONTINO |
| **-FEIT** | 飛行、傳送 | LOKTOFEIT |

### 一個範例：拆解 MADALTO

- **MA-**（加強）
- **DA-**（冰）
- **-ALTO**（霜）
- 意義：**「加強的霜冰術」** → **暴雪 8d8 群傷**

完美對應遊戲效果。

---

<a name="mage"></a>
## 二、魔法師咒語（Mage Spells, 21 個）

魔法師（**Mage**）、武士（**Samurai**，6 級起）、主教（**Bishop**）可學。
**每升 2 級學一級咒語**（武士升 4 級才開始學）。

<a name="mage-1"></a>
### Level 1（4 個）

#### HALITO ★ — 小火焰

| 屬性 | 值 |
|---|---|
| 詞源 | **HA**（火）**+ -LITO**（光） — 「**小火舌**」 |
| 效果 | 對單敵 1d8 火傷 |
| 戰鬥內外 | **僅戰鬥內** |
| 公式 | `1d8` （**期望值 4.5**） |
| 戰術 | B1F 對單個 Bubbly Slime 一發秒殺。1 級魔法師的**起手禮**。 |

#### MOGREF — 體鐵

| 詞源 | **MO-**（變形）**+ GREF**（鐵）  — 「**化身為鐵**」 |
|---|---|
| 效果 | 術者 AC -2 |
| 戰鬥內外 | 內外皆可 |
| 戰術 | 法師肉盾化的第一步。配合 SOPIC 可堆到 AC -6。 |

#### KATINO ★★ — 壞空氣

| 詞源 | **KA-**（窒息）**+ -TINO**（睡眠） |
|---|---|
| 效果 | 對一群敵人**睡眠**，**未抗性必中** |
| 公式 | 抗性檢定：**(20 × 等級)% 機率抵抗** |
| 戰術 | **Wizardry I 最強的 1 級咒語**——讓 Murphy's Ghost、Vampire、Greater Demon 等**頂級怪物全部秒睡**。 |
| 經典 | 「KATINO 大法好」——全 Wizardry 史上**最濫用**的咒語，後來在 W4 被官方降到 30%。 |

#### DUMAPIC ★ — 視線揭示

| 詞源 | **DU-**（讓）**+ -MAPIC**（看穿） |
|---|---|
| 效果 | 顯示「**第 N 層，X 北、Y 東，朝向 Z**」 |
| 戰鬥內外 | **僅戰鬥外** |
| 戰術 | 1980 年代你**唯一**的座標來源——畫地圖必備。本專案有 Eye-of-Map 後可選擇性使用。 |

<a name="mage-2"></a>
### Level 2（2 個）

#### DILTO — 黑暗

| 詞源 | **DI-**（？）**+ LTO**（陰影） |
|---|---|
| 效果 | 一群敵人 AC **+2**（**變差** = 你打他容易） |
| 公式 | 持續 1 戰 |
| 戰術 | 對 Vampire（AC 0）施 DILTO 後 AC +2 = 2，戰士命中率大增。 |

#### SOPIC — 玻璃化

| 詞源 | **SO-**（？） **+ -PIC**（影像） |
|---|---|
| 效果 | 術者 AC **-4**（**自身變透明**） |
| 戰術 | 配合 MOGREF（-2）= **法師 AC -6**，比戰士還難打到。 |

<a name="mage-3"></a>
### Level 3（2 個）

#### MAHALITO ★★ — 大火

| 詞源 | **MA-**（加強）**+ HALITO** |
|---|---|
| 效果 | 對一群敵人 **4d6 火傷** |
| 公式 | `4d6` （**期望值 14，最大 24**） |
| 戰術 | B3F–B6F **主力清群咒語**。 |

#### MOLITO — 火花

| 詞源 | **MO-**（小型）**+ -LITO** |
|---|---|
| 效果 | 對一群敵人 **3d6 物理傷害**（**非火屬性，不能被火抗抵消**） |
| 戰術 | 對 Fire Giant / 火元素**比 MAHALITO 有用**。 |

<a name="mage-4"></a>
### Level 4（3 個）

#### DALTO — 暴雪

| 詞源 | **DA-**（冰）**+ -ALTO**（霜） |
|---|---|
| 效果 | 一群敵人 **6d6 冰傷** |
| 公式 | `6d6`（期望 21，最大 36） |

#### LAHALITO ★ — 巨火

| 詞源 | **LA-**（至高） **+ HALITO** |
|---|---|
| 效果 | 一群敵人 **6d6 火傷** |
| 戰術 | DALTO 跟 LAHALITO 同等級同傷害——**選哪個看敵人弱火還弱冰**。 |

#### MORLIS — 恐懼

| 詞源 | **MO-**（變化）**+ -RLIS**（懼） |
|---|---|
| 效果 | 一群敵人 AC **+3** |
| 戰術 | 比 DILTO 強，比 MAMORLIS 弱。 |

<a name="mage-5"></a>
### Level 5（3 個）

#### MADALTO — 霜冰王

| 詞源 | **MA-** + **DALTO** |
|---|---|
| 效果 | 一群敵人 **8d8 冰傷** |
| 公式 | `8d8`（期望 36，最大 64） |
| 戰術 | 比 TILTOWAIT 弱但 MP 便宜。**B7F–B9F 主力**。 |

#### MAKANITO ★★ — 致命氣息

| 詞源 | **MA-** + **KANITO**（窒息） |
|---|---|
| 效果 | **直接殺死** 8 HD（hit dice）以下的非不死敵人 |
| 戰術 | **B5F–B8F 神級咒語**——一發清掉整群 Lvl 3 Mage / Bishop。**對 Vampire 無效**（不死）。 |
| 限制 | 不對 8 HD 以上敵人造成任何效果（**Werdna 免疫**）。 |

#### MAMORLIS — 全恐懼

| 效果 | **全部** 敵人 AC **+4** |
|---|---|

<a name="mage-6"></a>
### Level 6（4 個）

#### LAKANITO — 真空

| 詞源 | **LA-** + **KANITO** |
|---|---|
| 效果 | 立即殺死無抗性的敵人（**抗性檢定：6 × HD%**） |
| 戰術 | 比 MAKANITO 強，**對 Werdna 仍無效**（高 HD 抗性）。 |

#### ZILWAN ★★★ — 驅散

| 詞源 | **ZIL-**（聖）**+ -WAN**（散） |
|---|---|
| 效果 | 對**單個**不死生物 **10d200 傷害** |
| 公式 | `10d200`（**期望 1010**，最大 2000） |
| 戰術 | **完全為 Vampire / Vampire Lord / Lifestealer 設計**——**一發秒殺**。Werdna 隨從的 4 個 Vampire 用兩發 ZILWAN 解決。 |

#### MASOPIC — 水晶化

| 效果 | **全隊** AC **-4** |
|---|---|
| 戰術 | 進入 B10F 前必開。 |

#### HAMAN — 乞憐

| 詞源 | **HA-**（？）**+ -MAN**（祈求） |
|---|---|
| 效果 | **隨機 7 種效果之一**（治療 / 解狀態 / 滅敵 / 減魔抗） + **術者老化 1 年** |
| 戰術 | 賭博咒語。緊急時可能救你，但**老化是永久的**——年紀大會降屬性。 |

<a name="mage-7"></a>
### Level 7（3 個）

#### MALOR ★★ — 傳送

| 詞源 | **MA-** + **LOR**（位置） |
|---|---|
| 效果 | 戰鬥外：傳送到指定座標。戰鬥內：傳送到**隨機座標**（可能傳到牆裡 = **全隊滅團**）。 |
| 戰術 | **唯一回 B10F 起點的方式**——避免重走 9 層。**戰鬥中千萬別用**。 |

#### MAHAMAN — 大乞憐

| 詞源 | **MA-** + **HAMAN** |
|---|---|
| 效果 | **隨機 7 種強力效果**（清狀態 / 全隊滿血 / 殺敵 / 召喚） + **老化 1 年** |
| 戰術 | 比 HAMAN 強很多，但**只能戰鬥外用**。 |

#### TILTOWAIT ★★★★ — 滅世

| 詞源 | **TIL-**（時間）**+ TO-**（朝）**+ -WAIT**（等等！） |
|---|---|
| 字面譯 | 「**等等！**」（字面是停頓詞，但結果是滅世） |
| 效果 | **全部** 敵人 **10d15 物理傷害** |
| 公式 | `10d15`（**期望值 80**，最大 150） |
| 戰術 | **W1 最強單發咒語**——B10F 進 Werdna 房**第一回合必放**，清掉 4 個 Vampire 隨從。 |
| 經典 | 「TILTOWAIT 出，全場暴雪」——本咒語 1981 年首發時讓所有 RPG 玩家震驚：**一發咒語打整隊敵人 80 點傷害**，在當時不可思議。 |

---

<a name="priest"></a>
## 三、牧師咒語（Priest Spells, 30 個）

牧師（**Priest**）、領主（**Lord**，4 級起）、主教（**Bishop**）可學。

<a name="priest-1"></a>
### Level 1（5 個）

#### KALKI — 祝福

| 詞源 | **KAL-**（鋼）**+ -KI**（能量） |
|---|---|
| 效果 | **全隊** AC **-1** |

#### DIOS ★ — 治療

| 詞源 | **DI-**（生）**+ -OS**（祝福） |
|---|---|
| 效果 | 單體回 **1d8 HP** |
| 公式 | `1d8`（**期望 4.5**） |
| 戰術 | 1 級牧師的標配——對應 BADIOS。 |

#### BADIOS — 傷害

| 詞源 | **BA-**（毒）**+ DIOS**（治療反面） |
|---|---|
| 效果 | 對單敵 **1d8 物理傷害** |
| 戰術 | 牧師唯一的 1 級攻擊咒。MP 別浪費——交給戰士打就好。 |

#### MILWA ★ — 光明

| 詞源 | **MI-**（光）**+ -WA**（散） |
|---|---|
| 效果 | **15–29 步** 的迷宮照明（顯示隱藏門） |
| 戰術 | B3F 暗門密集區必背。 |

#### PORFIC — 護盾

| 詞源 | **PO-**（守）**+ -RFIC**（盔） |
|---|---|
| 效果 | 術者 AC **-4** |

<a name="priest-2"></a>
### Level 2（4 個）

#### CALFO — 透視

| 效果 | 鑑定寶箱陷阱類型，**95% 準確** |
|---|---|
| 戰術 | **比盜賊 INSPECT 還準**——B7F 之後寶箱陷阱會殺人，必背。 |

#### MANIFO — 雕像化

| 效果 | 一群敵人 **麻痺** |
|---|---|

#### MATU — 熱誠

| 效果 | **全隊** AC **-2** |
|---|---|

#### MONTINO ★★★ — 靜空氣

| 詞源 | **MON-**（無）**+ -TINO**（睡眠/靜默） |
|---|---|
| 效果 | 一群敵人**靜默**，**無法施法** |
| 公式 | 抗性：10 × HD% |
| 戰術 | **Wizardry I 神級防禦咒**——對 Arch Mage / Werdna 必放。**v3.2 已修原版 bug**。 |

<a name="priest-3"></a>
### Level 3（4 個）

#### LOMILWA ★★ — 永光

| 詞源 | **LO-**（長）**+ MILWA** |
|---|---|
| 效果 | **10,000 步** 照明 |
| 戰術 | 進新層必放。 |

#### DIALKO — 柔軟

| 效果 | 解除單體**睡眠 / 麻痺** |
|---|---|

#### LATUMAPIC ★ — 真名

| 詞源 | **LA-**（至高）**+ TU-**（你）**+ -MAPIC**（看） |
|---|---|
| 效果 | 看清怪物真名（從 `MAN` 變 `VAMPIRE LORD`） |
| 注意 | **原版有 bug 失效**，**v3.2 已修**。本專案 v0.6 沿用 v3.2 版本。 |

#### BAMATU — 大祈

| 效果 | **全隊** AC **-4** |
|---|---|

<a name="priest-4"></a>
### Level 4（4 個）

#### DIAL — 治療 II

| 效果 | 單體回 **2d8 HP**（期望 9） |
|---|---|

#### BADIAL — 傷害 II

| 效果 | 單敵 **2d8 傷害** |
|---|---|

#### LATUMOFIS — 解毒

| 詞源 | **LA-** + **TU-** + **-OFIS**（淨化） |
|---|---|
| 效果 | 解除中毒狀態 |
| 戰術 | Gas Cloud / Gas Dragon / Mace of Poison 中毒救命。 |

#### MAPORFIC — 大護盾

| 效果 | **全隊** AC **-2** 直到下次回城 |
|---|---|
| 戰術 | 進迷宮第一個動作。**唯一持久型 AC buff**。 |

<a name="priest-5"></a>
### Level 5（6 個）

#### DIALMA — 治療 III

| 效果 | 單體回 **3d8 HP**（期望 13） |
|---|---|

#### BADIALMA — 傷害 III

| 效果 | 單敵 **3d8 傷害** |
|---|---|

#### LITOKAN — 火焰

| 詞源 | **LITO-** + **KAN**（柱） |
|---|---|
| 效果 | 一群敵人 **3d8 火傷** |

#### KANDI — 定位

| 效果 | 顯示**離隊角色位置** |
|---|---|
| 戰術 | 角色在迷宮中死亡掉隊時找屍體用。 |

#### DI ★★ — 復活

| 詞源 | **DI-**（生） |
|---|---|
| 效果 | 復活屍體狀態的角色，**消耗 1 點 Vitality**（永久） |
| 失敗率 | 與術者等級、目標年齡有關，年紀大失敗率高 |
| 戰術 | 比 Temple 復活便宜，但**牧師等級不夠會失敗 → 屍體變灰燼**。**5 級以下牧師不建議用**。 |

#### BADI ★ — 死亡指

| 詞源 | **BA-**（殺）**+ -DI**（生命反面） |
|---|---|
| 效果 | 立即殺死單個敵人，**抗性：10 × HD%** |
| 戰術 | **對 Werdna 仍無效**（高 HD），但對 Bishop / Arch Mage 有用。 |

<a name="priest-6"></a>
### Level 6（4 個）

#### LORTO — 利刃

| 效果 | 一群敵人 **6d6 物理傷害** |
|---|---|

#### MADI ★★ — 完全治癒

| 詞源 | **MA-** + **DI** |
|---|---|
| 效果 | 單體**回滿 HP**，**解除中毒/麻痺/睡眠** |
| 戰術 | 6 級牧師的萬能藥。 |

#### MABADI ★★ — 致殘

| 詞源 | **MA-** + **BADI** |
|---|---|
| 效果 | 對單敵：**HP 減到 1d8**（**最終 HP 1–8**） |
| 戰術 | **對 Vampire Lord / Werdna 極有用**——一發把 60 HP Werdna 削到 1d8。 |
| 限制 | 對部分 Boss 有抗性。 |

#### LOKTOFEIT ★ — 回城

| 詞源 | **LOK-**（路）**+ TO-**（向）**+ -FEIT**（飛） |
|---|---|
| 效果 | 全隊瞬間回 Trebor 城堡 |
| 失敗率 | 2 × 等級 %（v3.2 修正前**全失敗**，**現已修好**） |
| 戰術 | 緊急脫困萬用咒。 |

<a name="priest-7"></a>
### Level 7（4 個）

#### MALIKTO — 神怒

| 詞源 | **MA-** + **LIK-**（光）**+ -TO** |
|---|---|
| 效果 | **全部** 敵人 **12d6 傷害** |
| 公式 | `12d6`（**期望 42**，最大 72） |
| 戰術 | 牧師版 TILTOWAIT（弱一些）。 |

#### KADORTO ★ — 灰燼復活

| 詞源 | **KA-**（無）**+ -DORTO**（生命複返） |
|---|---|
| 效果 | 從**灰燼狀態**復活（DI 失敗會變灰燼），消耗 1 點 Vitality |
| 戰術 | 救灰燼角色的**唯一**咒語。 |

#### MOGATO — 全隊魔抗

| 詞源 | **MO-**（隱蔽）**+ -GATO** |
|---|---|
| 效果 | 全隊獲得**魔法抵抗**，本場戰鬥所有敵方法術命中率降低 |
| 戰術 | 進 B10F 看到 Greater Demon 或 High Wizard 群時開場第一個下，可以擋掉一半 TILTOWAIT 級災難。對純物理敵（Frost Giant 等）浪費。 |

#### LABADI — 奪命

| 詞源 | **LA-**（巨大）**+ BADI**（傷害） |
|---|---|
| 效果 | **指定單體**，造成「**等於該目標當前 HP 的傷害**」 |
| 公式 | `dmg = target.hp_left`，命中後幾乎必殺 |
| 戰術 | 對單個高 HP boss 的處決術（如 Greater Demon 15d8、Vampire Lord 15d8）。但命中率不高，且**只對一個目標**；對群敵不如 MALIKTO。 |

---

<a name="v32-fix"></a>
## 四、v3.2 修正過的咒語

snafaru v3.2 修了 4 個原版的咒語 bug：

| 咒語 | 原版問題 | v3.2 修正 |
|---|---|---|
| LATUMAPIC | 從不生效 | 正確顯示怪物真名 |
| LOKTOFEIT | 100% 失敗 | 失敗率改為 2 × 等級 % |
| MONTINO | 抗性算錯（怪物**從不**被靜默） | 改為 10 × HD % 抗性 |
| ZILWAN | 對非不死也生效 | **僅對 undead** |

本專案 v0.6 沿用所有 v3.2 修正。
v1.22 後 **LATUMAPIC 真正讓戰鬥日誌全程顯示真名**（`display_name(g)` helper），
**MONTINO 靜默機制** 全戰鬥週期阻止敵方施法，**CALFO** 改為偵測迷宮陷阱（看前方一格 SquareFeature 並命名）。
v1.22 同時引入**法術槽自動補滿/扣除**（`recompute_spell_slots()` Mage/Priest/Bishop/Samurai/Lord 五系規則 + cap 9），
詠唱前 slot gate；沒槽就「**法力已盡**」、不消狀態、不發效果。投宿任何房型即補滿。

---

<a name="combos"></a>
## 五、實戰咒語組合

### Combo 1：B1F 新手保命

`PORFIC` → `MILWA` → `KATINO` → 戰士狂砍 → `DIOS`

### Combo 2：B6F+ 進迷宮開場

`LOMILWA` → `MAPORFIC` → `MATU`（**全隊 AC -8**）

### Combo 3：對 Arch Mage / Werdna

`MONTINO`（**強制靜默**）→ `MABADI`（**HP 砍到 1d8**）→ 戰士砍

### Combo 4：對 Vampire 群

`ZILWAN` × 2（**每發 10d200 秒殺一個 Vampire**）

### Combo 5：B10F 最後一戰開場

回合 1：
1. **TILTOWAIT**（清掉 4 Vampires）
2. **MONTINO**（Werdna 靜默）
3. **MABADI**（Vampire Lord）
4. 全隊砍 Werdna

### Combo 6：滅團前救命

`MADI`（牧師單回滿血）→ `LOKTOFEIT`（全隊回城）

---

<a name="sources"></a>
## 六、引用來源

- [Wizardry Wiki (wiki.gg) — Spells](https://wizardry.wiki.gg/wiki/Spells)
- [Wizardry I-IV Spells (Fandom)](https://wizardry.fandom.com/wiki/Wizardry_I-IV_spells)
- [StrategyWiki — Wizardry 1 Spells](https://strategywiki.org/wiki/Wizardry:_Proving_Grounds_of_the_Mad_Overlord/Spells)
- [DungeonCrawl-Classics — Wizardry 1 Spells Database](https://dungeoncrawl-classics.com/wizardry-series/proving-grounds-of-the-mad-overlord/wizardry-1-spells/)
- [Snafaru — Wizardry 1-2-3 Game Calculations](https://www.zimlab.com/wizardry/walk/wizardry-123-game-calculations.htm)
- [Data Driven Gamer — The not-so-basic mechanics of Wizardry](https://datadrivengamer.blogspot.com/2019/08/the-not-so-basic-mechanics-of-wizardry.html)
- 本專案 `assets/data/spells.json`
- Sir-tech《Ultimate Wizardry Archives》Manual, 1998 Interplay 再版, p52–58
