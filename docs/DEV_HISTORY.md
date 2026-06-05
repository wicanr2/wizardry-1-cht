# 開發編年史 — v0.3 到 v1.7 的設計、踩坑與抉擇

> **為什麼讀這篇？**
> 這份文件不是給玩家的——是給**未來想要做類似專案的開發者**的「**踩雷地圖**」。
> 從 2026 年 6 月 3 日的 v0.3「首次發佈」，到 6 月 5 日 v1.7c「F1 help overlay」——
> **整個專案在 72 小時內推進了 15 個版本**。
> 這篇老老實實寫下每個版本**為何這樣做、踩到什麼坑、怎麼跳過去**，
> 讓下一個想用 C++17 + SDL2 重寫古典 RPG 的人，能少花一些晚上。

---

## 目錄

1. [為何選 C++17 + SDL2](#why-cpp)
2. [v0.3 — 首次發佈：訓練場 + 城鎮 + Eye-of-Map](#v03)
3. [v0.4 — PCE-CD 立繪 + Camp 存檔](#v04)
4. [v0.5 — Roller lore + Inn 房型對齊手冊](#v05)
5. [v0.6 — 法術系統補完（51 咒語）](#v06)
6. [v0.7 — W6 .pic 格式研究](#v07)
7. [v0.8 — Apple II LZ 解壓 toolchain](#v08)
8. [v0.9 — 翻譯覆蓋率 91%](#v09)
9. [v1.0 — GitHub Actions CI matrix](#v10)
10. [v1.1–v1.5 — Sprite 全齊與視覺驗證](#v11-v15)
11. [v1.6–v1.7 — 音效系統與 Help overlay](#v16-v17)
12. [踩過最痛的坑](#worst-pits)
13. [未來方向](#future)
14. [致謝](#thanks)

---

<a name="why-cpp"></a>
## 一、為何選 C++17 + SDL2

### 替代方案的評估

開工前，我評估過以下技術棧：

| 方案 | 優點 | 缺點 | 結論 |
|---|---|---|---|
| **Python + pygame** | 開發快 | 慢、字型 CJK 麻煩、難分發 | ❌ |
| **Rust + ggez** | 安全、現代 | 學習曲線、生態不成熟 | ❌ |
| **TypeScript + Electron** | UI 容易 | 100MB+ 體積、重 | ❌ |
| **Go + Ebiten** | 簡單 | 字型渲染弱、CJK 支援差 | ❌ |
| **Lua + LÖVE2D** | 輕量 | 不容易跨平台分發 | ❌ |
| **C++17 + SDL2** | **跨平台、字型一流、輕量、效能** | 編譯設定麻煩 | ✅ |

### 為什麼非 C++17 不可

1. **SDL2 + SDL_ttf 是 CJK 字型渲染的事實標準**——
   Noto Sans CJK TC 14000 字符可以**全部 hinting + anti-aliasing**
2. **靜態連結 = 單檔分發**——Windows 用戶不用裝 Python / Node.js
3. **跨平台 = 1 套 CMakeLists.txt 處理 Linux / Windows / macOS**
4. **執行檔體積 < 5 MB**，加上資產 < 30 MB 全平台 release
5. **效能**：1981 年的遊戲在 2026 年的硬體上**不該超過 1% CPU**

### 為什麼是 C++17 不是 C++20/23

- C++17 是 **gcc 8 / clang 5 / MSVC 19.14** 起完整支援，**Ubuntu 18.04+ 預裝可用**
- C++20 modules 還沒到位（**MSVC 2026 才完整**）
- coroutine / concept 對本專案沒有必要

---

<a name="v03"></a>
## 二、v0.3 — 首次發佈：訓練場 + 城鎮 + Eye-of-Map

**Release**: 6 月 3 日（commit `09df6e1`）
**字串覆蓋**: 35%（初版翻譯）

### 完成內容

- 標題畫面
- 城鎮邊緣（Edge of Town）+ 4 個城堡子場景
  - Boltac's Trading Post（**波塔克貿易站**）
  - Gilgamesh's Tavern（**吉爾伽美什酒館**）
  - Adventurer's Inn（**冒險者旅館**）
  - Temple of Cant（**坎特神殿**）
- 訓練場（Roller）8 步創角流程
- **Eye-of-Map 自動繪圖**
- B1F 一層迷宮（20×20）
- 基礎戰鬥（F/S/P/R 四選項）

### 設計抉擇

#### 為何 1280×720 而不是原版 280×192

- 1981 年 Apple II 是 **40 column × 24 row text mode + 一條 HGR**
- 直接放大到 1280×720 = **每格 32 像素** = **CJK 字型可清楚顯示**
- 16:9 比例 = **跟 2020 年代的螢幕對齊**

#### 為何 Eye-of-Map 是「藍色填、白線」

當時試過 4 種配色：

| 配色 | 評價 |
|---|---|
| 綠底白線（matrix 風） | 太復古 |
| 黑底黃線（GameBoy 風） | 對比太強 |
| 灰底黑線（紙地圖風） | 看不清 |
| **藍底白線**（**SDL2 預設 cornflowerblue**） | **舒服、識別度高** ✅ |

---

<a name="v04"></a>
## 三、v0.4 — PCE-CD 立繪 + Camp 存檔

**Release**: 6 月 4 日（commit `faa931b`）

### 完成內容

1. **55 個 PCE-CD 怪物立繪**，從 [wizardry.wiki.gg](https://wizardry.wiki.gg) 抓取（CC-BY-SA 4.0）
2. **Camp 系統**：在迷宮中可休息、檢視狀態、存檔
3. **存檔格式**：`~/.config/wizardry-cht/save.json`

### 抓取工具：`tools/fetch_pcecd_sprites.sh`

```bash
#!/bin/bash
# 一鍵下載 55 張 PCE-CD 立繪
for sprite in "${SPRITES[@]}"; do
    curl -L "https://wizardry.wiki.gg/images/.../${sprite}" \
         -o "assets/sprites/monsters_pcecd/${sprite}"
done
```

### 踩坑：wizardry.wiki.gg 的 URL 結構

第一次抓取**失敗**——wiki.gg 的圖片 URL 不是簡單的 `wiki/Image:Xxx.png`，
而是 MediaWiki 的 `images/hash/...` 結構：

```
https://wizardry.wiki.gg/images/0/0a/PCECD_MS_Bubbly_Slime.png
```

`hash/0/0a` 是檔名 MD5 的前 2 個字元——**必須先查每張圖的 hash**。
解法：先抓 wiki 頁面解析 HTML，**從 `<img src="...">` 摳出真實 URL**。

### Camp 存檔格式設計

考慮過 3 種：

| 格式 | 評價 |
|---|---|
| Binary (struct dump) | 平台依賴、難 debug | ❌ |
| YAML | 漂亮、但 C++ 庫笨重 | ❌ |
| **JSON (nlohmann/json)** | **C++17 header-only、漂亮、易讀** | ✅ |

---

<a name="v05"></a>
## 四、v0.5 — Roller lore + Inn 房型對齊手冊

**Release**: 6 月 4 日（commit `3e16cba`）

### 完成內容

1. **Roller 選種族 / 職業時顯示 tooltip**——抄自 Sir-tech 手冊 p11–15
2. **Inn 房型修正**：原版的「Cots」誤譯為「簡易床位」，**改為手冊正名**：
   - **Stables**（**馬廄**——免費，HP 不回）
   - **Barracks**（**兵營**——50 GP/天）
   - **Double Occupancy**（**雙人房**——200 GP/天）
   - **Private Room**（**單人房**——500 GP/天）
   - **Royal Suite**（**皇家套房**——1500 GP/天）

### 踩坑：手冊與遊戲不一致

對照 1998 Interplay 再版手冊 vs. snafaru v3.2 程式碼，發現**8 處不一致**：

| 項目 | 手冊 | 程式碼 |
|---|---|---|
| Inn 房型名 | 5 種 | 5 種，但譯名與手冊不同 |
| Bishop 入門需求 | I.Q. 12 + Piety 12 | 一致 |
| Ninja 入門需求 | 全屬性 17+ | **snafaru v3.2 改為 15+** |
| Boltac 鑑定費 | 50% | 一致 |

**結論**：以**手冊為翻譯依據**、以**程式碼為機制依據**。本專案 v0.5 採用此原則。

---

<a name="v06"></a>
## 五、v0.6 — 法術系統補完（51 咒語）

**Release**: 6 月 4 日（commit `838091a`）

### 完成內容

把 **51 個咒語**全部接到戰鬥引擎與 Camp 介面。

### 設計抉擇：咒語表如何寫

選項 A：把 51 條 `if (name == "HALITO") ...` 寫進 C++ 程式碼——**簡單但難維護**。

選項 B：在 `assets/data/spells.json` 定義 metadata，C++ 讀取後 dispatch——**較好擴充**。

**選了 B**：

```json
{
  "name": "TILTOWAIT",
  "school": "mage",
  "level": 7,
  "type": 1,
  "group": 1,
  "combat_only": true,
  "description": "滅世，10d15 群體破壞"
}
```

`type` / `group` 是**法術行為分類**：

- `type: 0` = 純效果（buff/debuff/狀態）
- `type: 1` = 傷害
- `type: 2` = 控制（睡眠/麻痺/靜默/恐懼）
- `group: 1` = 火/冰/暴雪群傷家族
- `group: 11` = 全隊 AC buff
- ……

### 踩坑：v3.2 修正的咒語邏輯

snafaru v3.2 修了 4 個原版咒語 bug：

| 咒語 | 原版 | v3.2 修正 |
|---|---|---|
| LATUMAPIC | 從不顯示真名 | 正確顯示 |
| LOKTOFEIT | 100% 失敗 | 失敗率 2×等級 % |
| MONTINO | 抗性算錯 | 改為 10×HD % |
| ZILWAN | 對非不死也生效 | 僅對 undead |

**全部沿用 v3.2 版本**。

---

<a name="v07"></a>
## 六、v0.7 — W6 .pic 格式研究

**Release**: 6 月 4 日（commit `ba77950`）

### 動機

`docs/REFERENCES.md` 提到 Interplay ISO 裡有 **59 個 W6 BANE 怪物的 `.pic` 檔**——
**EGA 320×200 16 色**，可以當**替代立繪源**。

### 完成內容

- `docs/W6_PIC_FORMAT.md` — 格式分析筆記
- `tools/analyze_w6_pic.py` — 嘗試解壓的 Python 腳本

### 結論：未破解的格式

`.pic` 的前 16 byte 是 header，後面是某種自製 LZ 壓縮——
**Sir-tech 1990 年的內部格式，沒有公開規範**。
即使比對了 5 個 .pic 檔，**也找不到一致的解壓邏輯**。

**暫時擱置** — v0.4 PCE-CD 立繪已經夠用。

---

<a name="v08"></a>
## 七、v0.8 — Apple II LZ 解壓 toolchain

**Release**: 6 月 4 日（commit `e1ad458`）

### 動機

從 `release/wiz1.zip` 抽出 1981 Apple II 原版的 **title screen art**——
**280×192 HGR**，比 PCE-CD 立繪更**原汁原味**。

### 完成內容

- `tools/lzdecomp.py` — Apple II UCSD Pascal LZ 解壓器（**self-implemented**）
- `tools/hgr_to_png.py` — Apple II HGR 格式轉 PNG

### 踩坑：HGR 的 7-bit 詭異設計

Apple II HGR 模式是**每 byte 7 bit 像素 + 1 bit 色相位**：

```
byte[0] bit 0–6 = 7 個像素，bit 7 = 色彩相位
byte[1] bit 0–6 = 下 7 像素，bit 7 = 色彩相位
...
```

而且**色彩模型不是 RGB**——是「**4 個基色（黑 / 白 / 紫 / 綠 / 橙 / 藍）**」 + **相位影響**。
這比 EGA / VGA 都複雜，因為它是**完全 hack 出來的硬體限制**——
**Steve Wozniak 1977 年為了省 chip 設計出來的**。

最後**只成功解了 title screen**，maze art 因為 LZ 壓縮邏輯不一樣**還在研究**。

---

<a name="v09"></a>
## 八、v0.9 — 翻譯覆蓋率 91%

**Release**: 6 月 5 日（commit `d87763e`）

### 完成內容

- `tools/extract_strings.py` 掃出 **454 個 unique strings**
- `assets/i18n/zh_TW.json` 翻譯 **413 條 = 91%**

### 漏網之魚（剩 9%）

| 類別 | 數量 | 例 |
|---|---|---|
| Debug strings | 12 | `"DEBUG: combat state"` |
| Error messages | 8 | `"FATAL: spell name not found"` |
| Unreachable strings | 6 | dead code 殘留 |
| Constants | 15 | `"WIZARDRY"` 標題 logo |

**有意不翻**：debug / error 保留英文便於 bug report；logo 保留英文是品牌。

---

<a name="v10"></a>
## 九、v1.0 — GitHub Actions CI matrix

**Release**: 6 月 5 日（commit `27e6836`）

### 完成內容

- `.github/workflows/build.yml`：**Linux + Windows matrix build**
- `.github/workflows/sanity.yml`：translation coverage + JSON 驗證
- **Automated release**：tag `v*` 推送時**自動產生 release artifact**

### 踩坑 1：vcpkg pkgconf 404

```
Error: Could not download https://www.freedesktop.org/.../pkgconf-2.x.x.tar.xz
404 Not Found
```

`vcpkg` 的 `pkgconf` 包依賴 freedesktop.org 的 tarball——**該 URL 在 2026 年 5 月某日失效**。

**解法**：把 vcpkg 釘到 `2026.06.01` 後修好的版本（commit `88c6842`）。

### 踩坑 2：GitHub Actions binary cache

vcpkg 預設用 GitHub Actions binary cache 加速——
**但需要 `ACTIONS_CACHE_URL` token**，**fork 倉庫沒有**。
**解法**：禁用 binary cache（commit `8ed5e51`）——慢但能跑。

### 踩坑 3：sanity job 路徑

第一版 CI 的 sanity job 用相對路徑找 `assets/i18n/zh_TW.json`，**fail on Windows**。
**解法**：改為 `${{ github.workspace }}/assets/...` 絕對路徑。

---

<a name="v11-v15"></a>
## 十、v1.1–v1.5 — Sprite 全齊與視覺驗證

### v1.1 — Sprite 對應 monsters.json

把 60 個 PCE-CD 立繪打包進 repo（**不抓 wiki 了**），**對應 `monsters.json` 的 30 個怪物**。

### v1.2 — Camp 法術接 11 個 out-of-combat 咒語

把 DIOS / MADI / DUMAPIC / MILWA / DI / KADORTO / LATUMOFIS / DIALKO / LOKTOFEIT / KANDI / LATUMAPIC
**接到 Camp 介面**，讓玩家**真的可以用牧師咒語自療**。

### v1.3 — SDL_mixer + Kevin MacLeod BGM

**4 軌音樂**：
- `title.mp3` — Kevin MacLeod "Achaidh Cheide" (CC-BY 4.0)
- `town.mp3` — "Cool Vibes"
- `maze.mp3` — "Ascending the Vale"
- `combat.mp3` — "Heroic Age"

**踩坑**：SDL_mixer 在 Windows 上**依賴 dynamic load OGG decoder**——
**第一版 release 缺 `libvorbisfile.dll` 跑不出聲**。
**解法**：在 CMakeLists.txt 用 `vcpkg install sdl2-mixer[mpg123,vorbis]`。

### v1.4 — 30/30 Sprite 全齊

補完最後一個沒有 sprite 的怪物（Master Ninja 第 2 個變體）。**完成度 100%**。

### v1.5 — `docs/v15_all_30_sprites.png`

寫了個 snapshot 工具：**進戰鬥 → 換 30 隻怪物 → 截圖拼貼**——
產出單張**1820×1080 的全 30 隻立繪拼貼圖**，作為視覺回歸測試 baseline。

---

<a name="v16-v17"></a>
## 十一、v1.6–v1.7 — 音效系統與 Help overlay

### v1.6 — 程序生成音效

**11 個 SFX**，全部用 **ffmpeg 程序合成**（不需要授權聲源檔）：

| 音效 | 合成方式 |
|---|---|
| `attack.wav` | 100 ms sine sweep 200→50 Hz |
| `spell_cast.wav` | 300 ms FM modulation |
| `step.wav` | 80 ms noise burst |
| `door_open.wav` | 500 ms low-freq creak |
| `monster_die.wav` | 400 ms descending sine |
| `party_damage.wav` | 150 ms square wave 80 Hz |
| `menu_select.wav` | 50 ms click |
| ... | |

合成腳本在 `assets/audio/sfx/synthesize.sh`。

### v1.7 — F1 Help overlay

按 F1 顯示**情境式幫助**：

- 城鎮畫面 → 顯示城堡子場景按鍵說明
- 迷宮畫面 → 顯示 WASD + 法術說明
- 戰鬥畫面 → 顯示 F/S/P/R 戰術提示

詳見 `docs/v17_help_castle.png` / `_maze.png` / `_combat.png`。

### v1.7c — Windows CMake CONFIG mode

**踩坑**：MSVC build 失敗：

```
CMake Error: Cannot find SDL2_ttf-config.cmake
```

原因：vcpkg 的 SDL2_ttf 用的是 **CONFIG mode**，不是 MODULE mode。
**解法**：CMakeLists.txt 寫：

```cmake
find_package(SDL2_ttf CONFIG REQUIRED)
target_link_libraries(wizardry_cht PRIVATE SDL2_ttf::SDL2_ttf)
```

---

<a name="worst-pits"></a>
## 十二、踩過最痛的坑

### Pit #1：xdotool focus 偷不到

在 Linux 上**程序化截圖**（用於 docs/）需要 `xdotool windowactivate` 把 SDL 視窗拉到前景——
但 **GNOME 3.38+** 拒絕讓 xdotool 取得焦點（**安全策略**）。

**解法**：改用 `wmctrl` + `xwd`（X11 native dump）。

### Pit #2：vcpkg pkgconf 404

已述。

### Pit #3：Camp 法術系統設計

最初把法術 dispatch 寫在戰鬥引擎裡，**Camp 介面沒辦法重用**。
**解法**：把法術 effect 抽成 `SpellEffect` 物件，**戰鬥引擎與 Camp 都呼叫它**。

### Pit #4：CJK 字型 anti-aliasing

SDL_ttf 預設關閉 anti-aliasing——**Noto Sans CJK TC 看起來像 1990 年代位元圖**。
**解法**：用 `TTF_RenderUTF8_Blended` 而不是 `TTF_RenderUTF8_Solid`。

### Pit #5：JSON 反序列化 enum

`monsters.json` 把 `lvl` 寫成 `int`，但 `level` 在某些怪物是字串 `"10"`——
**nlohmann/json 預設不寬鬆**——**fail loudly**。
**解法**：寫 helper：

```cpp
int parse_lvl(const json& j) {
    if (j.is_number()) return j.get<int>();
    return std::stoi(j.get<std::string>());
}
```

### Pit #6：Eye-of-Map 與迷宮 wrap-around 衝突

第一版 Eye-of-Map 假設 `(x, y)` 是 (0..19, 0..19)，**沒處理 wrap-around**——
從 `19E` 走一步變 `0E` 後，**地圖上會出現一條跨整條的線**。
**解法**：繪圖前 `x = (x + 20) % 20`。

---

<a name="future"></a>
## 十三、未來方向

### 短期（v1.8–v2.0）

- [ ] **B2F–B10F 迷宮資料**——目前只有 B1F；其他 9 層需要從原版 byte-level rip
- [ ] **完整 100 怪物**——目前 30 隻；剩 70 隻需要 PCE-CD 立繪或自繪
- [ ] **完整道具庫**（含 Werdna's Amulet 詛咒鎖定機制）
- [ ] **macOS build**——CI matrix 還沒加 macOS

### 中期（v2.x）

- [ ] **盜賊寶箱小遊戲**——目前自動成功；應該加上機率 + 失敗陷阱觸發
- [ ] **酒館點餐 + 角色互動**——手冊 p16 有提到，未實作
- [ ] **NPC 對話系統**——B4F 之後可能有 Trebor 傳令使者

### 長期（v3.x+）

- [ ] **W2 Knight of Diamonds 接續**——同樣的 engine，加 W2 資產
- [ ] **W3 Legacy of Llylgamyn**
- [ ] **網頁版（emscripten 編譯）**——讓不會編譯的人也能玩

### 不會做

- ❌ **3D 透視渲染**——保留 wireframe 是設計選擇（**懷舊 + 計算便宜**）
- ❌ **語音對白**——Wizardry 沒有對白，加了不對味
- ❌ **多人連線**——這是 PvE 遊戲，不需要

---

<a name="thanks"></a>
## 十四、致謝

- **snafaru** — Wizardry.Code v3.2 修正版的 45 年累積
- **Thomas William Ewers** — Wizardry I Pascal/Assembly 原始反組譯
- **4am** — v3.2 字型修正
- **Qkumba** — RNG 與記憶體安全修正
- **Andrew Greenberg & Robert Woodhead** — 1981 年的兩位 Cornell 學生
- **Sirotek 家族** — Sir-tech 公司
- **Kelly R. Flewin** — GameFAQs 攻略
- **wizardry.wiki.gg 社群** — CC-BY-SA 4.0 立繪庫
- **Kevin MacLeod** — CC-BY 4.0 配樂
- **Noto Sans CJK 字型團隊**（Google + Adobe）— OFL 1.1 字型
- **台灣 1985 年 Apple II 玩家** — 用口耳相傳保留了 Wizardry 的中文記憶

> *「程式碼會老，但故事不會。Trebor 還在等他的護身符；我們只是讓他用中文等。」*

---

## 引用來源

- 本專案 `git log`（commits `09df6e1` 到 `9458392`）
- [snafaru/Wizardry.Code](https://github.com/snafaru/Wizardry.Code)
- [vcpkg release notes 2026.06.01](https://github.com/microsoft/vcpkg/releases/tag/2026.06.01)
- [Kevin MacLeod incompetech.com](https://incompetech.com/)
- [SDL2 documentation](https://wiki.libsdl.org/SDL2/FrontPage)
- 本專案 `docs/REFERENCES.md`, `docs/W6_PIC_FORMAT.md`, `docs/A2_TITLE_EXTRACTION.md`
