# Wizardry VI Bane of the Cosmic Forge —（巫術 VI：宇宙鍛爐之災）`.PIC` 格式研究筆記

## 概況

W6 (1990) 的 monster portraits（怪物頭像） / 場景畫面用 `.PIC` 副檔名儲存。
Sir-tech（賽爾科技）從未公開格式規格，社群也沒有公開的 decoder（解碼器）。
本文件記錄目前的 reverse-engineering（逆向工程）觀察，供未來繼續推進。

## 檔案清單

`Wizardry.iso /BANE/` 下：

| 類型 | 檔案 | 數量 | 用途 |
|------|------|------|------|
| 怪物頭像 | `MON00.PIC` ~ `MON58.PIC` | 59 | 戰鬥畫面 |
| 場景 | `DRAGONSC.{CGA,EGA,T16}` | 3 | 龍場景 cutscene |
| 場景 | `GRAVEYRD.{CGA,EGA,T16}` | 3 | 墓地 |
| 迷宮 | `MAZEDATA.{CGA,EGA,T16}` | 3 | 迷宮繪圖素材 |
| 製作 | `CREDITS.PIC` | 1 | 製作群 |
| 顯示卡驅動 | `CGA.DRV` / `EGA.DRV` / `HERC.DRV` | 3 | 顯卡專用渲染 |

```
副檔名意義（File extension semantics）：
- .PIC   = 主要 EGA (Enhanced Graphics Adapter, IBM 增強型顯示卡) 格式 (最常見)
- .CGA   = CGA (Color Graphics Adapter, IBM 彩色顯示卡) 4 色備援
- .EGA   = EGA 16 色 (常常和 .PIC 重複?)
- .T16   = Tandy 1000 (1984 Radio Shack 推出的家用 PC 相容機) 16 色
```

## 觀察到的 pattern

跑 `tools/analyze_w6_pic.py` 得到的統計：

```
name             size   ed00   first16
mon00.pic        1166     21   025802fd01ed00027802fd01ed000298
mon01.pic        4469     17   0658020305ff7fee000638040305ff7f
mon02.pic        8973      5   0958020706ff9fefe7e3f10006580604
mon05.pic        2184     23   055802020103ed00059802020103ed00
mon09.pic       20462      2   075802020a75550def000cd803050cce
mon13.pic       23451      1   1558020b0c0080033ff8811f7ce0031f
```

### 確定的觀察

1. **第二 byte 永遠是 0x58 (十進位 88)**
   - 59 個檔案全部如此
   - 可能是 magic number（檔案魔術數字 / 格式識別碼）或固定的 width hint（寬度提示）

2. **第三 byte 永遠是 0x02**
   - 也是常數

3. **第一 byte 不固定，範圍 0x02 ~ 0x15**
   - 跟檔案大小無強相關
   - 可能是 record count（記錄數量） / 起始 Y 座標 / 高度

4. **`ED 00` 出現次數變化很大**
   - mon05: 23 次（小檔案）
   - mon09: 2 次（大檔案）
   - 不是行尾標記 — 可能是某種 escape 序列（跳脫序列）或子記錄結尾

5. **byte 0xFF / 0x00 / 0xFD / 0xFC 出現最頻**
   - 典型 EGA bitplane（位元平面）資料特徵
   - 0xFF = 8 pixels 全 on，0xFD = pattern `11111101`
   - 強烈暗示這是 **EGA 4 bitplane**（4 個位元平面組成 16 色，每平面 1 bit/pixel） 結構

### 推測格式

以下是 hypothesis（假設）的 C 結構（並未驗證能渲染出正確圖像）：

```
struct PicHeader {
  uint8  unknown_count;   // 首位元組 (範圍 0x02..0x15)
  uint8  magic1 = 0x58;
  uint8  magic2 = 0x02;
  // ... 後續為記錄陣列
};

// 每個記錄可能是：
struct PicRecord {
  uint8  flags_or_x;        // flags 旗標或 X 座標
  uint8  y_or_count;        // Y 座標或記錄數量
  uint8  bitplane_data[N];  // 每像素 1 bit × 4 個位元平面
  // 部分記錄結尾可能是 0xED 0x00
};
```

但這只是 hypothesis（假設），沒有可以渲染出正確圖像的驗證。

## 為何難解

1. **EGA driver（顯示卡驅動）在外部**：`EGA.DRV` 是分開的二進位（8.8KB）。
   渲染邏輯在 driver 內，PIC 只是資料 + 解釋器指令（interpreter opcodes）。
2. **Sir-tech 自製格式**：跟標準 PCX / GIF / IFF（PC Paintbrush / Graphics Interchange Format / Interchange File Format，三種常見圖檔）都不相容
3. **無原始碼**：W6 source（原始碼）未公開（不像 W1 有 snafaru 的 Pascal）
4. **平台特化**：CGA/EGA/T16 三套不同 binary（執行檔）暗示有平台特化的編碼路徑

## 建議的下一步

優先順序由低成本到高成本：

### Phase 1：靜態分析（半天）
1. 反組譯 `EGA.DRV`（disassemble，將機器碼還原為組合語言）找渲染入口點
2. 找出讀取 PIC 的 syscall（系統呼叫）與其 buffer layout（緩衝區記憶體布局）
3. 對照 mon00 / mon01 兩個小檔案，逐 byte trace（追蹤每個位元組）

### Phase 2：動態觀察（1-2 天）
1. 在 DOSBox（開源 DOS 模擬器）跑 W6，breakpoint（中斷點）下在 EGA.DRV 渲染函式
2. dump VGA framebuffer（傾印 VGA 影格緩衝區）跟 PIC 對比
3. 找出 "pixel byte → screen byte" 的映射函式（mapping function）

### Phase 3：渲染器實作（2-3 天）
1. 用 Python + numpy 寫 prototype renderer（原型渲染器）
2. 對所有 59 個 PIC 跑一次，跟 W6 in-game screenshot（遊戲畫面截圖）對比
3. 移植到 C++ 整合進本專案

## 替代方案

由於格式複雜度高、ROI（Return on Investment，投資報酬率）不確定，
建議**短期內**使用 PCE-CD sprite 套（已完成，CC-BY-SA 授權）作為主要美術來源。

W6 PIC 解碼留作未來 v2.0 milestone（里程碑）。

## 社群資源

如果有人想接力：
- [r/wizardry](https://reddit.com/r/wizardry) — Reddit 系列粉絲社群（subreddit）
- [datadrivengamer.blogspot.com](https://datadrivengamer.blogspot.com) — 已研究多代 Wizardry mechanics（遊戲機制）
- [archive.org Wizardry section](https://archive.org/details/wizardry) — 含 W1 source（原始碼）但 W6 沒有
- DOSBox debugger（除錯器） + Ghidra（NSA 開源逆向工具） / IDA（Interactive Disassembler，商業逆向工具）— 標準逆向工具

---

研究工具：`tools/analyze_w6_pic.py`
原始素材：`release/Wizardry.iso/BANE/MON*.PIC`（不打包進 repo）
