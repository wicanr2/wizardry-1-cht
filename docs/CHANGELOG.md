---
layout: default
title: 完整變更紀錄 — v0.2 → v1.25
---

# 完整變更紀錄

> 從 2026-03 的 v0.2 城鎮系統雛形，到 v1.25 三平台 ship — 22 個版本的全部里程碑。
> README 的 hero 段只列當前狀態；本檔保留完整 changelog 給想看開發脈絡的讀者。
> 開發踩坑詳細記錄見 [`DEV_HISTORY.md`](DEV_HISTORY.md)。

| 項目 | 狀態 |
|------|------|
| 全部里程碑 (M1~M8) | **8 / 8 完成** |
| v0.2 城鎮系統 | 訓練場 / 商店 / 旅館 / 神殿 全部 done |
| v0.3 進階功能 | 酒館編隊 / 戰鬥法術 / **Eye-of-Map** / Camp 存檔 done |
| v0.4 美術升級 | **55 個 PCE-CD 怪物立繪** (CC-BY-SA from wizardry.wiki.gg) |
| v0.5 手冊對校 | Roller lore tooltip + Inn 房型對齊 Sir-tech 手冊 |
| **v0.6 法術完整** | **全 51 咒語接 effect**（HALITO ~ TILTOWAIT，含 v3.2 修正） |
| v0.7 W6 .pic 研究 | docs/W6_PIC_FORMAT.md 格式分析筆記 |
| v0.8 Apple II LZ（Lempel-Ziv 1977 字典式壓縮，原版用來壓 title 畫面） | LZDECOMP + HGR（Apple II 高解析繪圖緩衝區）→PNG Python toolchain |
| **v0.9 翻譯** | **100% 中文化**（453/453 keys，含原版版權聲明逐條中譯） |
| **v1.0 CI** | **GitHub Actions Linux + Windows matrix，自動 release** |
| **v1.1 Sprite 對應** | **60 個 PCE-CD sprite bundled，29/30 怪物有專屬立繪** |
| **v1.2 Camp 法術** | **11 個 out-of-combat 咒語**（DIOS/MADI/DUMAPIC/MILWA/DI...） |
| **v1.3 BGM** | **SDL_mixer + Kevin MacLeod CC-BY 4 軌**（title/town/maze/combat） |
| **v1.4 Sprite 全齊** | **30/30 怪物有 PCE-CD 立繪** + snapshot 視覺驗證工具 |
| v1.5 視覺驗證 | docs/v15_all_30_sprites.png — 全 30 隻 in-game 拼貼 |
| **v1.6 SFX** | **11 個程序生成音效**（攻擊/法術/腳步/選單/開門...） |
| **v1.11 多 slot 存檔** | **5 槽位**，標題畫面 1–5 鍵直接讀檔 + Camp 槽位選擇器 |
| **v1.11 GitHub Pages** | [文件站上線](https://wicanr2.github.io/wizardry-1-cht/)，Cayman theme |
| **v1.12 F3 主題切換** | **PCE-CD / Mono / Outline / Sepia** + 各 theme 可獨立 BGM |
| **v1.13 規則 gap fix** | Poisoned 狀態（tick 扣血/Castle 解毒）+ 沉睡/麻痺 gating + 前/後排陣型 |
| **v1.14 迷宮陷阱** | **4 種陷阱**：Pit / Spinner / Teleporter / Chute 全部生效 |
| **v1.14 轉職系統** | Camp [X] 鍵 — 8 職業屬性 + 陣營雙重檢核 |
| **v1.15 F4 多語** | **繁中 / English / 日本語** 全局即時切換 |
| **v1.16 多層迷宮** | **10 層 B1F-B10F**，[原版地圖](https://wicanr2.github.io/wizardry-1-cht/MAPS.html)轉錄完成 |
| **v1.16 永久死亡** | 戰死屍體留在迷宮，需新隊伍走回原格拾取 |
| **v1.17 神殿復活** | DI（亡→生）+ KADORTO（灰→生）含失敗風險與 Status::Lost 終局 |
| **v1.18 日文化完成** | **453/453 UI 條目全部補上 ja_JP 欄位**（F4 三語切換 100% 涵蓋） |
| **v1.19 遭遇率對齊** | per-step `enmy_calc[0]` formula `30 + 7*(L-1)` clamp 110 — B1F ~12% / B10F ~43%，B1F 走完 smoke test |
| **v1.20 主題深化** | F3 切換時迷宮牆/門/地板/天花板/自動地圖**全部跟著換色**；標題背景也走 theme-aware fallback；UI chrome 字串改走 tr() |
| **v1.21 戰鬥真實感** | 突襲回合（先發/被突襲）+ 吸血鬼吸等級 + 食屍鬼麻痺 + 龍類吐息（vit-save 半傷）|
| **v1.22 缺口收束** | 法術槽自動補滿/扣除（Mage/Priest/Bishop 延遲）+ Afraid +2 AC + 神殿解咒 + 完整 8 種陷阱 + CALFO/LATUMAPIC 真正生效 |
| **v1.23 P2 polish** | M 鍵自動地圖開關 + 北/東/南/西 + 深度指北針 + Shift+↑↓ 隊伍重排 + Camp 匯出角色卡 + Dark zone（無光僅見 1 格） |
| **v1.24 結局畫面** | 沃登納 B10F 戰勝 → `Scene::Ending` PCE-CD 風格合成圖（OVA 城堡 + 金色護符 + 倒下的瘋王 + 暗角暈影）+ 三語結局文 |
| **v1.24 一鍵打包** | Linux `wizardry-cht-x86_64.AppImage` 單檔執行 + Windows `wizardry-cht-windows-x64.zip` 含全 DLL + `.bat` launcher |
| **v1.25 戰鬥重排** | 1981 原版風 — 3D 迷宮當背景 + 怪物中央浮現 + 我方右側 + AC/HP 半透明條 |
| **v1.25.1 sprite 透明** | 61 PCE-CD master + Mono + Sepia 共 ~78 萬像素黑底 alpha-key → 怪物站在 3D 走廊裡正確透明 |
| **v1.25.2 QA 修正** | game-tester 60-min Xvfb 找到 14 issue，這版修 5 個 P0/P1：EdgeOfTown ESC 不再 quit / Shop sell+identify / klass 顯示「戰士」非「fighter」/ Tavern Shift+↑↓ 修正 / ✦→» 字型 fallback |
| **v1.25.3 F10 + ESC iron-law** | ESC 永遠只 cancel/back / F10 = 唯一退出鍵 / 跳 Yes/No modal + 自動存 Slot 1 才離開 |
| 法術 / 怪物 / 道具表 | 51 法術 + 30 怪物 + 30 道具，含中文名 |
| 單元測試 | **14 / 14 ctest 全綠**（含 v13 mechanics + class_change + special_attacks + spell_slots） |
| 平台 | Linux x86_64 / Windows 10+ / macOS arm64 |
| 字型 | Noto Sans CJK TC（思源黑體繁中，OFL 1.1 開源字型授權 / SIL Open Font License，可打包） |
| GitHub | [wicanr2/wizardry-1-cht](https://github.com/wicanr2/wizardry-1-cht) |
| 📖 文件站 | [wicanr2.github.io/wizardry-1-cht](https://wicanr2.github.io/wizardry-1-cht/) — 怪物 / 法術 / 道具 / 攻略 / 開發史 + STRATEGY + LORE + TEST_REPORT 11 篇繁中知識庫 |

---

*完整每個版本的設計決策、踩過的坑、未來方向見 [`DEV_HISTORY.md`](DEV_HISTORY.md)。*
