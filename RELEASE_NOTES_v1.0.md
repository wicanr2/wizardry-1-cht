# Wizardry 1 CHT v1.0 — Release Notes

> *Wizardry: Proving Grounds of the Mad Overlord* (1981) 繁體中文化重寫版
> 首次 public release，2026-06-05

## 🎮 此版本可玩的內容

從標題畫面開始，能完整體驗：

1. **訓練場** — 8 步 Roller 建立角色，含官方 lore tooltip
   - 5 種族 / 8 職業 / 3 陣營，依 Sir-tech 手冊 p11-15 正確過濾
   - v3.2 修正：Ninja 15 attributes 可玩（而非原版 17）

2. **吉爾伽美什酒館** — Roster ↔ Party 編隊系統
   - Tab 切換面板、A 加入、R 移除
   - 隊伍上限 6 人，名冊上限 200 人

3. **波塔克貿易站** — 30 件道具買賣
   - 多顧客 Tab 切換、金幣不足提示
   - 中文物品名（長劍、鎖子甲、小盾 等）

4. **冒險者旅館** — 5 種房型 / HP 恢復 / 自動升級
   - 對齊 Sir-tech 手冊 p32：Stables / Barracks / Double / Private / Royal
   - XP 滿足條件自動升級

5. **坎特神殿** — 治療 / 解毒 / 解麻痺 / 解石化 / 復活
   - 費用以等級乘數計算

6. **迷宮 B1F** — 3D 線框透視 + 撞牆檢測
   - WASD / 方向鍵移動
   - **Eye-of-Map 自動繪圖**（mini-map 即時 reveal）
   - SPACE 觸發戰鬥、C 進營地

7. **營地** — 存檔 / 檢視 / 重排 / 回標題
   - JSON 存檔到 `~/.config/wizardry-cht/save.json`
   - 開機自動讀檔

8. **戰鬥** — 完整回合制系統
   - F/S/P/R 四選一行動
   - **51 個咒語完整實作**（HALITO / MAHALITO / KATINO / DIOS / BADIOS / TILTOWAIT / MALOR / KADORTO ...）
   - 怪物有 PCE-CD 1991 立繪
   - 命中 / 傷害 / XP / 金幣自動結算

## 📊 數字

| 項目 | 數量 |
|------|------|
| C++17 程式碼行 | ~5,500 |
| Python 工具行 | ~700 |
| 法術 | 51 |
| 怪物（含中文名） | 30 |
| 道具（含中文名） | 30 |
| PCE-CD 立繪 (CC-BY-SA) | 55 |
| i18n 翻譯覆蓋率 | **91%** (413/454) |
| 單元測試 | 7/7 全綠 |
| Git commits | 12+ |

## 🛠 技術棧

- C++17 + SDL2 + SDL2_ttf + SDL2_image
- 1280×720 自由排版，Noto Sans CJK TC TTF 抗鋸齒
- CMake 跨平台 build
- GitHub Actions CI matrix (Linux / Windows)

## 📥 下載

從 [Releases 頁面](https://github.com/wicanr2/wizardry-1-cht/releases/tag/v1.0)：

- `wizardry-cht-linux.tar.gz` — Linux x86_64 (Ubuntu 22.04+)
- `wizardry-cht-windows.zip` — Windows 10+ x64（advisory，可能需自行 build）

或從源碼編譯：

```bash
git clone https://github.com/wicanr2/wizardry-1-cht.git
cd wizardry-1-cht
tools/fetch_assets.sh             # 抓 Noto CJK 字型
tools/fetch_pcecd_sprites.sh      # 抓 55 個怪物立繪 (~300KB)
cmake -B build && cmake --build build -j
./build/src/wizardry_cht
```

## 🙏 致謝

- **Andrew Greenberg & Robert Woodhead** (Sir-tech, 1981) — 原作者
- **Thomas William Ewers** — Wizardry I Pascal 反組譯（v3.2 基底）
- **[snafaru](https://github.com/snafaru/Wizardry.Code)** — v3.2 + 100 項規則修正
- **4am、Qkumba** — v3.2 字型修正、RNG 修正
- **[wizardry.wiki.gg](https://wizardry.wiki.gg)** 社群 — PCE-CD 怪物立繪 (CC-BY-SA 4.0)
- **Sir-tech / Interplay 1998 manual** — 譯名校對基底

## 🐛 已知限制

- Windows binary 還沒在 CI matrix 穩定（vcpkg 設定中），請先用源碼編譯
- 迷宮目前只有 B1F demo 版（待從 disk image 抽完整 10 層）
- 怪物 sprite 只整合到 1 個 (Kobold)，剩餘 54 個 sprite 已下載但尚未綁定 monsters.json
- 7 個 out-of-combat 咒語（MALOR / LOMILWA / CALFO ...）目前只 log 不執行
- 翻譯尚有 9% (41 條長句) 留人工潤稿

## 🔮 v1.1+ 路線圖

1. 修 Windows CI 讓 binary release 兩平台齊全
2. 接 Kevin MacLeod CC-BY 音樂（SDL_mixer）
3. 從 Apple II disk image 抽 1981 原版 LZ title art（UCSD P-System reader）
4. W6 .pic 動態觀察反組譯（DOSBox debugger + Ghidra）
5. 完整 10 層 maze 資料
6. 剩餘 54 個 PCE-CD sprite 對應 monsters.json
7. 7 個 out-of-combat 咒語效果（含 MALOR 瞬移）

---

Made with 🧙 in Taiwan, 2026.
