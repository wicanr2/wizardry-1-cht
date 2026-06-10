---
layout: default
title: 官方參考資料清單
---

# 官方參考資料清單

收錄使用者於 2026-06-04 補進的 Sir-tech 官方文獻與遊戲檔案。這些檔案
**未納入本 repo**（受版權限制），但翻譯與機制驗證會以它們為準。

---

## 1. Sir-tech（賽爾科技）《Ultimate Wizardry Archives》（巫術終極典藏集）官方手冊 PDF

**檔案**: `Ultimate Wizardry Archives_ Game Manuals - Sir-Tech.pdf` (6.8 MB)
**版本**: Interplay（卬特普雷遊戲公司）1998 年再版的合集手冊，內含 W1〜W7 全套
**頁數**: 259 頁

### 對本專案直接相關章節（W1: Proving Grounds, p5–58）

| 頁碼 | 章節 | 翻譯參考價值 |
|------|------|------|
| 11 | Characters / Statistics | 6 屬性的官方解釋 |
| 11 | Race | 5 種族的官方個性描述（用於 Roller 的種族 tooltip） |
| 11 | Alignment | Good/Neutral/Evil 的官方 NPC 反應描述 |
| 12 | Possessions / Cursed items | 受詛咒道具機制 |
| 13 | Experience / Class | 等級 / 職業切換規則 |
| 14–15 | 8 個職業詳細 lore | 戰士 / 魔法師 / 牧師 / 盜賊 / 主教 / 武士 / 領主 / 忍者 |
| 15 | Spells | 法術系統（mage/priest, 7 級, slot 制） |
| 16–17 | The Castle / Boltac / Temple / Inn | 城堡子場景介紹 |
| 18 | The Mission / Trebor / Werdna | 主線劇情官方描述 |
| 22 | Class Entry Requirements | **正式表格**（已用於我們的 rules.cpp） |
| 28–32 | Boltac / Temple / Inn 詳細操作 | 已整合進 v0.2/v0.3 |
| 33–36 | Edge of Town / Camp | Camp 系統機制 |
| 52+ | Spells (完整 51 法術) | 法術參考 |

### 重要 lore（背景設定）摘錄（用於翻譯校對）

英文原文後附中譯：

> **Humans** excel at nothing, but have no particular weaknesses, except for a decided lack of piety.
> **人類**沒有特長，但也沒有明顯的弱點，只是出名地缺乏信仰心。

> **Elves** are intelligent and pious, but not very robust. They excel at intellectual pursuits, and are excellent spell-casters.
> **精靈**聰明且虔誠，但體格不甚強健。擅長智識追求，是優秀的施法者。

> **Dwarves** are strong and hardy. They love fine weapons and armor and delight in a good fight.
> **矮人**強壯而堅韌，喜愛精良的武器與盔甲，樂於戰鬥。

> **Gnomes** are pious and agile, probably from praying underground during earthquakes.
> **地侏**虔誠且敏捷，大概是從地震中於地下祈禱而練出來的。

> **Hobbits** are agile and very, very lucky.
> **哈比人**敏捷，而且非常、非常幸運。

> **Trebor**, the Mad Overlord, was not always mad, merely power crazed... He had obtained a fabled amulet, whose awesome powers were the subject of many legends... Werdna had used the amulet to carve out of the ground below the Castle a ten level fortress maze...
>
> **瘋王特雷波**並非一開始就瘋，只是熱衷權勢……他取得了一只傳說中的護身符，其力量是諸多傳說的源頭……沃登納用這護身符在城堡下方挖鑿出一座 10 層的要塞迷宮……

> **Note**: "Wizardry is a non-sexist game, however, English is a sexist language."
> **註**：「Wizardry 不是個性別歧視的遊戲，只不過英語是有性別歧視的語言。」
> （1981 年作者的自我吐槽，可作為譯註保留原意。）

### 翻譯方針確認

依手冊 p11 與全章用詞，本專案譯名標準如下：

| 英文 | 中譯 | 來源 |
|------|------|------|
| Strength / I.Q. / Piety / Vitality / Agility / Luck | 力量 / 智力 / 信仰 / 體力 / 敏捷 / 幸運 | 手冊 p11 |
| Maze | 迷宮 | 手冊 p18 |
| Castle | 城堡 | 手冊 p16 |
| Boltac's Trading Post | 波塔克貿易站 | 手冊 p16 |
| Gilgamesh's Tavern | 吉爾伽美什酒館 | 手冊 p16 |
| Temple of Cant | 坎特神殿 | 手冊 p17 |
| Adventurer's Inn | 冒險者旅館 | 手冊 p17 |
| Trebor, the Mad Overlord | 瘋王特雷波 | 手冊 p18 |
| Werdna | 沃登納 | 手冊 p18（音譯，待人工確認） |
| amulet | 護身符 | 手冊 p18 |

---

## 2. Apple II（蘋果二號電腦）原版磁碟（1981）

**檔案**: `release/wiz1.zip` (131 KB)
**內含**:
- `wizardry_i_boot.dsk` (143360 bytes / 35 track × 16 sector × 256 byte，即 35 軌 × 16 磁區 × 256 位元組)
- `wizardry_i_scenario.dsk` (143360 bytes)
**格式**: Apple DOS 3.3（蘋果 3.3 版磁碟作業系統）但實際是 **UCSD Pascal P-System**（加州大學聖地牙哥分校 Pascal 虛擬機）格式（與 snafaru repo 的 Wiz1A〜E.DSK 同源但更早）
**版本**: 推估 2012 年從 1981 原版 mastered（母片製作）的 archival rip（典藏抽取版）

### 用途

- 比對 snafaru v3.2 與原版 byte-level 差異（驗證 100+ fix 各自動了哪幾 byte）
- 抽取原版 LZ 壓縮的 title screen art（280×192 HGR）
- 確認 1981 年最早的怪物名 / 道具名拼寫

### 抽取工具

```bash
# 安裝 ciderpressii (或用 ApplePy/python)
pip install python-apple-disk
# 或用 a2tools:
apt install a2tools
a2tools ls wizardry_i_scenario.dsk
```

---

## 3. Interplay（卬特普雷遊戲公司）《Ultimate Wizardry Archives》（巫術終極典藏集）CD ISO

**檔案**: `release/Wizardry.iso` (662 MB)
**版本**: 1998-11-02 Interplay 出版的 6 合 1 合集
**格式**: ISO 9660（國際標準的光碟檔案系統），磁區 2048 bytes

### 目錄結構

| 路徑 | 內容 | 大小 |
|------|------|------|
| `MANUALS/MANUAL.PDF` | 同上述合集手冊 | 7 MB |
| `WIZARD15/WIZ{1-5}.DSK` | W1–W5 DOS 版磁碟映像 | 320 KB × 5 |
| `WIZARD15/WIZ{1-5}.COM` | DOS 啟動器 | ~2-19 KB |
| `WIZARD15/SAVE{1-5}.DSK` | W1-5 預存檔 | 640 KB × 5 |
| `BANE/MON{00-31}.PIC` | W6 怪物立繪（59 個 .pic 檔） | 1-23 KB 每張 |
| `BANE/DRAGONSC.{CGA,EGA,T16}` | W6 龍場景動畫 | 16-32 KB |
| `BANE/GRAVEYRD.{CGA,EGA,T16}` | W6 墓地場景 | 16-32 KB |
| `BANE/MAZEDATA.{CGA,EGA,T16}` | W6 迷宮素材 | 52-102 KB |
| `BANE/CREDITS.PIC` | W6 製作群 PIC | 20 KB |
| `DSAVANT/` | W7 Crusaders of the Dark Savant | – |
| `WIZGOLD/` | Wizardry Gold（W7 Windows 強化版） | – |
| `DEMOS/` | 1998 年其他 Interplay 遊戲試玩 | – |

### WIZ1.DSK 技術細節

```
$ xxd wiz1.dsk | head -2
00000000: eb38 9080 0800 0000 0053 7973 7465 6d20  .8.......System
00000010: 626f 6f74 7374 7261 7020 7632 2e30 2044  bootstrap v2.0 D
                                                    SDD..(C) 1987 RWI
```

- `eb 38 90` = 8086 jump instruction（Intel 8086 處理器的跳躍指令，標誌開機磁區起點）
- OEM string（OEM 字串，Original Equipment Manufacturer 廠商識別字串）：
  "System bootstrap v2.0 DSDD (C) 1987 RWI, Inc" = Sir-tech 1987 自製 boot loader（開機載入器）
- 320 KB（雙面雙密 5.25 吋磁片，DSDD = Double-Sided Double-Density）
- **不是 FAT12**（DOS/Windows 標準的 12 位元檔案配置表）— 是 UCSD Pascal P-System 格式（與 Apple II 共用 codebase 原始碼基底）
- mtools（Linux 上讀寫 FAT 磁片的工具集）無法直接讀，需要 P-System aware（識得 P-System 的）工具

### Wizardry VI Bane（巫術 VI：宇宙鍛爐之災）怪物 PIC 格式

```
$ ls bane/mon*.pic | wc -l
59
$ file bane/mon00.pic
mon00.pic: data
```

- 1990 年 EGA（Enhanced Graphics Adapter，IBM 增強型顯示卡）320×200 16 色格式（自製壓縮）
- 對應 PC EGA 顯示卡
- 大小 1-23 KB 不等（依複雜度）
- **未鑑定的解壓格式**（社群 RHDN [ROM Hacking dot Net，老遊戲改造社群] 可能有 ripper [抽取工具]）

### 法律提醒

⚠️ 整片 ISO 屬於 **Interplay / Sir-tech / Drecom IP**（智慧財產權）：
- 自用研究 OK
- **不可重新打包** 進本專案 release
- 抽出的素材**不可進 git repo**
- 可在 docs 中**引用 manual 文字** 做翻譯比對（屬於 fair use，合理使用原則）

---

## 4. 整合計畫

### 短期（v0.5）

1. **手冊文字校對**：將 i18n catalogue 的 zh_TW 條目逐條對照 manual p5–58 修正
2. **Class lore Tooltip**：Roller 選職業時顯示手冊 p14–15 的官方描述（中譯）
3. **Inn 房型名稱對齊**：手冊用 *Stables / Barracks / Double Occupancy / Private Room / Royal Suite*，目前我們有 Cots 是錯的（已修為簡易床位）

### 中期（v0.6）

1. **抽 Apple II 原版 disk art**：寫 ProDOS reader + LZ decompressor，重現 280×192 HGR title
2. **驗證 v3.2 修正**：比對 1981 原版與 snafaru v3.2 的關鍵 bytes，確認 100+ 修正落實
3. **W6 BANE monster PIC reverse**：研究 .pic 格式，可作為 alternative monster art 來源

### 長期（v0.7+）

1. **OVA 截圖**：1991 Wizardry 1 OVA (50min) 抽 5–10 張當開場 cinematic 素材
2. **官方拼字校對**：以 Sir-tech 手冊為準，修正所有英文拼寫（如 `LATUMAPIC` 是否該為 `LATUMOFIS`、`Murphy` 的所有格寫法等）

---

## 5. 引用授權說明

引用 Sir-tech 手冊文字進入本專案 README / docs / 翻譯註解 → 屬 **Fair Use（合理使用）**：
- 教育性質（重寫研究 + 在地化）
- 引用比例有限（不是整段抄入）
- 不損及商業利益（原版不販售繁中）
- 附明確 attribution（資料來源標示，例：「來源：Sir-tech《Ultimate Wizardry Archives》Manual, 1998 Interplay 再版」）

引用具體機制數據（屬性閾值、HP 公式、XP 表）→ **不受版權保護**（屬於事實 / 算法，不在著作權保護範圍）。
