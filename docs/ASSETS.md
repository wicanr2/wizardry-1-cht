---
layout: default
title: Wizardry I 素材來源指南
---

# Wizardry I 素材來源指南

整理 *Wizardry: Proving Grounds of the Mad Overlord* 各版本的視覺/音樂素材包來源，
便於本專案後續從目前的「純文字 + ASCII 線框」升級到「有怪物立繪 + 真正美術」的版本。

---

## 1. 已在 repo 內的素材（snafaru/Wizardry.Code 上游）

```
Wizardry.Code/
├── resources/
│   ├── boot-splash-01.png          # v3.2 啟動畫面
│   ├── boot-splash-v3.2.png
│   ├── castle-market-party-03.png  # Apple II HGR (High-Resolution Graphics, 高解析繪圖) 截圖
│   ├── character-stats-02.png
│   └── maze-entrance-party-03.png
├── latest-disk-images-Wizardry.Code/
│   ├── Wiz1A.03-Aug-2023.DSK       # ProDOS disk images (蘋果專業作業系統磁碟映像)
│   ├── Wiz1B.23-Mar-2024.DSK       # 內含原 LZ (Lempel-Ziv) 壓縮的 title art + maze art
│   ├── Wiz1C.12-Feb-2026.DSK
│   └── Wiz1D.17-Mar-2026.DSK
└── latest-disk-images-System.Startup/
    └── Wiz1E.18-Mar-2026.DSK       # LZDECOMP.TEXT + title screen LZ data
```

這些 disk image（磁碟映像）含 1981 年 Apple II（蘋果二號電腦）原版的所有資料：
280×192 HGR title screen（高解析開機畫面）、
迷宮 vector lines（向量線段）、字型 ROM（內建字型唯讀記憶體）。
但格式是 ProDOS（Apple Professional DOS，蘋果專業作業系統）block + LZ 壓縮，需要工具抽出。

**抽取工具候選**：
- [CiderPress II](https://ciderpress2.com/) — ProDOS disk reader（磁碟讀取器，Windows/Mac）
- [ApplePy](https://github.com/jasonpepas/ApplePy) — Python 寫的 ProDOS library
- 自己寫 LZ decompressor（解壓器；LZDECOMP.TEXT 已有 Pascal/Assembly 原碼可對照）

---

## 2. Wizardry Wiki — PCE-CD 怪物 sprite（最易取得）

**🎯 推薦首選**：[wizardry.wiki.gg](https://wizardry.wiki.gg)

- URL: https://wizardry.wiki.gg/wiki/Category:Proving_Grounds_of_the_Mad_Overlord_PCECD_monster_sprites
- **94 個怪物 sprite（角色立繪）**，全部 64×96 像素 PNG
- **License: CC-BY-SA 4.0**（Creative Commons 創用 CC「姓名標示-相同方式分享 4.0 國際」：可商用、可修改、需署名 + 衍生作須以相同授權釋出）
- 1-5KB 每張，可直接點擊下載
- 包含：龍類（Fire / Gas Dragon，火龍 / 毒氣龍）、人形（各等級的 Fighter / Mage / Priest / Thief，戰士 / 魔法師 / 牧師 / 盜賊）、
  不死類（Vampire / Zombie / Ghost，吸血鬼 / 殭屍 / 幽靈）、巨人、惡魔、稀有怪（Vorpal Bunny 致命兔、Werdna 沃登納）

這套 sprite 是 1991 年 PC-Engine CD（NEC HuCard 主機 1988 上市，1991 推出 CD-ROM² 增訂版）版本的官方美術，
比 NES（任天堂 8 位元紅白機，日本稱 Famicom）版精細很多，
又有 CC-BY-SA 授權，是最適合 fan project（同人 / 粉絲開發專案）用的素材。

整合計畫：
```
assets/sprites/monsters/
  ├── 00_bubbly_slime.png
  ├── 01_orc.png
  ├── 02_kobold.png
  ...
  └── 25_werdna.png
```

`monsters.json` 已有 30 條 entry，依 id 對應即可。

---

## 3. Wizardry Wiki — 其他可用集合

| 集合 | 數量 | 用途 |
|------|------|------|
| [Return of Werdna (W4) PCECD enemy sprites](https://wizardry.wiki.gg/wiki/Category:The_Return_of_Werdna_PCECD_enemy_sprites) | 多 | W4 怪物也可借用，造型偏「邪惡視角」 |
| [Proving Grounds WSC monster sprites](https://wizardry.wiki.gg/wiki/Category:Proving_Grounds_of_the_Mad_Overlord_WSC_monster_sprites) | 多 | WonderSwan Color 版本，較像素風 |
| [Bane of the Cosmic Forge SFC class sprites](https://wizardry.wiki.gg/wiki/Category:Bane_of_the_Cosmic_Forge_SFC_class_sprites) | 多 | 隊員職業頭像（SFC = Super Famicom） |

---

## 4. 同類 open-source 重製專案（可借鑒）

### [davemoore22/sorcery](https://github.com/davemoore22/sorcery)

- 開源 C++ remake（重製），目標就是 W1 Proving Grounds（瘋王試煉場）
- 美術來源：
  - **Torio**（pixiv 藝術家 ID 5887541）— 怪物立繪
  - **game-icons.net** — 通用 UI 圖示
- 音樂：**Kevin MacLeod**（凱文・麥克勞德，獨立配樂家；incompetech.com，CC-BY 4.0 創用 CC 姓名標示授權）
- License: **GPL**（GNU General Public License，GNU 通用公眾授權；屬 copyleft 授權，衍生作須沿用同授權）（程式碼），美術另計
- 結構：`/gfx`（圖）、`/sfx`（音效）、`/vfx`（視覺特效）

> 注意：sorcery 是 GPL，本專案是 MIT。GPL 的 copyleft 條款會「感染」整個衍生作品，
> 若要直接引用其資產要評估授權衝突。

### Wizardry I OVA（Original Video Animation，原創影像動畫，1991）

- 50:43 分鐘官方動畫，TMS Entertainment（東京電影新社）製作
- archive.org（網際網路檔案館）: https://archive.org/details/wizardry-1-ova
- 330MB，MP4 480p
- 可截圖當參考素材或片頭/結尾用

---

## 5. 商業作品（不可直接用，但可參考）

| 作品 | 平台 | 特色 |
|------|------|------|
| **Digital Eclipse 2024 Remaster**（數位日蝕公司重製版） | Steam / Switch / PS / Xbox | 2024 年新美術，**保留原版 byte-level（位元組級別）邏輯**，內建 automap（自動繪圖） |
| **Wizardry: The Five Ordeals HD Sprites DLC**（五試煉 HD 立繪追加下載內容） | Steam | 150MB 高解析度 sprite pack（立繪包） |
| **PC-98 / FM Towns 日版**（NEC 個人電腦 9801 系列 / 富士通 FM Towns，1985–） | 日本電腦平台 | 日本風格美術重繪，已絕版 |
| **NES (1987 JP / 1990 NA)** | NES（任天堂紅白機 / Family Computer） | 8-bit sprite，可從 NES ROM 抽 |
| **SFC (1999)** | Super Famicom（任天堂超級紅白機） | 16-bit sprite |
| **GBC (1999)** | Game Boy Color（任天堂彩色掌機） | 簡化版美術 |
| **PS1 New Generation**（PlayStation 新世代） | PS1 (2001) | 偽 3D，日版獨佔 |

> **靈感參考**：Spriters Resource 有 [Wizardry V FM-Towns sprites](https://www.spriters-resource.com/fm_towns/wizardryvheartofthemaelstrom/) — 同系列風格延續。

---

## 6. 1980-1990 年代台灣相關出版物

從 u6-cht 專案的經驗：[電腦玩家雜誌](https://www.computer-gamer.com.tw/) 與軟體世界
在 1990 年代曾出版過 Wizardry 相關中文化資料：

- 怪物中譯名（電腦玩家《巫術 1 攻略本》，年份待查）
- 法術中譯（與 Ultima 系列共通的部分音節）
- 1992 年 BBS 玩家社群整理的職業/陣營對照表

這些資料**多數已絕版**，需要：
1. 找私人收藏
2. 國立圖書館的舊雜誌掃描
3. 老 BBS Archive (如 ptt.cc 的 wizardry 板，若還存在)

---

## 7. 音樂 / 音效

W1 原版 Apple II **沒有音樂**（只有 click sound）。可用素材：

| 來源 | 授權 | 風格 |
|------|------|------|
| [Kevin MacLeod](https://incompetech.com) | CC-BY 4.0 | 中世紀 / 奇幻 / 史詩 |
| [OpenGameArt.org](https://opengameart.org) | CC0 / CC-BY | 各種 |
| [FreeMusicArchive](https://freemusicarchive.org) | 多種 | 廣泛 |
| Wizardry NES soundtrack | 不可商用 | 8-bit 風格參考 |
| Wizardry PC-98 soundtrack | 不可商用 | 90s JP RPG |

---

## 8. 建議整合路徑

### Phase A：抽 PCE-CD 怪物 sprite（最快見效）

```bash
# 寫個 tools/fetch_pcecd_sprites.py:
# 1. 從 wiki.gg category 頁面抓 94 張 sprite 連結
# 2. 下載到 assets/sprites/monsters/
# 3. 依檔名 mapping 到 monsters.json 的 id
# 4. 戰鬥畫面顯示對應 sprite
```

預估工作量：**半天**。能讓戰鬥畫面立刻從文字升級到「有怪物頭」。

### Phase B：抽 Apple II 原版迷宮/title art

```bash
# 寫個 tools/extract_apple_disk.py:
# 1. 讀 Wiz1A.DSK ProDOS layout
# 2. 找到 LZ-compressed title screen
# 3. 用 LZDECOMP 演算法（已在 Pascal source 內）解壓
# 4. 280x192 HGR bitmap → PNG
```

預估工作量：**1-2 天**。能還原 1981 年原汁原味的開機畫面。

### Phase C：類比 sorcery 接 Kevin MacLeod 音樂

```bash
# 從 incompetech.com 挑：
# - 城鎮主題（如 "Tavern Loop One"）
# - 迷宮主題（如 "Lost Time"）
# - 戰鬥主題（如 "Dauntless"）
# - Werdna boss 主題（如 "Final Strike"）
```

預估工作量：**1 天**（含整合 SDL_mixer）。

---

## 法律 / 授權快速參考

| 來源 | 可商用？ | 需署名？ | 可改編？ | 可同梱進 release？ |
|------|---------|---------|---------|-------------------|
| wizardry.wiki.gg sprites (CC-BY-SA 4.0) | ✅ | ✅ | ✅ | ✅（要附 LICENSE 與 SA 條款） |
| sorcery 程式碼 (GPL) | ⚠️ | ✅ | ✅ | ⚠️ 整個專案要轉 GPL |
| Kevin MacLeod (CC-BY 4.0) | ✅ | ✅ | ✅ | ✅ |
| Apple II 原版 disk image | ❌ | – | ❌ | ❌ (Drecom IP) |
| Digital Eclipse 2024 美術 | ❌ | – | ❌ | ❌ |
| PCE-CD ROM 直接 rip | ❌ | – | ❌ | ❌ (廠商 IP) |

> ✅ = 直接拿來用；⚠️ = 要謹慎評估；❌ = 不要碰

---

**下一步建議**：先做 Phase A（PCE-CD 怪物 sprite），可以在一天內讓戰鬥畫面有質的飛躍。
