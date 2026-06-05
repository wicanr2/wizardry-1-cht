# Apple II Wizardry I — 1981 Title Screen Extraction
（蘋果二號電腦原版開機畫面抽取）

把原版 Apple II（蘋果二號電腦）的 280×192 HGR（High-Resolution Graphics，高解析繪圖模式）title screen（開機畫面）還原到 PNG。

## 目前進度

✅ **完成**：
- `tools/lz_decomp.py` — LZDECOMP.TEXT 的 Python 移植（原為 Apple II 6502 組合語言）
- `tools/hgr_to_png.py` — Apple II HGR framebuffer（影格緩衝區） → 280×192 PNG，含 NTSC artifact 色彩（NTSC 制式類比訊號邊緣偽彩，Apple II 利用此特性顯示「彩色」）
- `tools/extract_a2_title.py` — DSK（Apple II 磁碟映像檔）暴力掃描器

⚠️ **遇到瓶頸**：DSK 是 **UCSD Pascal P-System**（加州大學聖地牙哥分校 Pascal 系統的虛擬機格式） 格式，
title screen 的 LZ（Lempel-Ziv 1977，字典式壓縮）blob（壓縮資料區塊）不在簡單的 byte offset（位元組偏移），需要先解 P-System 目錄。

暴力掃 440 個 candidate offsets（候選偏移位置），最高 score 0.744 但渲染出來都是雜訊。

## 還原原理

snafaru repo 內 `TITLELOA.TEXT` 顯示（以下為原版 1981 年 UCSD Pascal 源碼，
留意 `(* ... *)` 是 Pascal 註解語法）：

```pascal
WTBUFF : ARRAY[ 0..4607] OF INTEGER;  (* 9 BLOCKS == 18 SECTORS *)

BEGIN
  LZDECOMP( WTBUFF[ 0], WTBUFF[ 8]);   (* 主 title *)
  ...
  LZDECOMP( WTBUFF[ 0], WTBUFF[ 0]);   (* 動畫 frame 1 *)
END;
```

說明：

- **WTBUFF 是 9216 bytes（18 sectors / 18 個磁區，每磁區 512 byte）**從磁碟 block 載入
- **`LZDECOMP(WTBUFF, offset)`** 在 buffer（緩衝區）內偏移處讀 LZ 流（compressed stream），
  直接寫進 HGR 緩衝區（記憶體位址 $2000-$3FFF，共 8KB）

所以正確做法：
1. 從 P-System filesystem（檔案系統）找出 title 相關 data file（資料檔）
2. 該 file 內容就是 WTBUFF[0..4607]
3. 依 `LZDECOMP(WTBUFF[0], WTBUFF[N])` 的偏移 N 解壓

## LZDECOMP 演算法（已實作）

從 6502 asm（6502 CPU 組合語言）移植。以下為 token 解析的中文對照：

```
游標指向的 token：
  byte < 0xC0  → row index（列號）；接著讀 1 byte mask（位元遮罩） + 最多 5 byte 字面資料
  0xC0..0xFC   → (col_offset = byte - 0xC0, row, value) 三元組
  0xFD         → XOR-flash effect（XOR 閃爍動畫；靜態畫面可忽略）
  0xFE         → scroll-left effect（向左捲動動畫；可忽略）
  0xFF / 其他  → end of stream（流結束）
```

關鍵點：寫入位置由 row 透過 HGR_LO[row] / HGR_HI[row] 查表得到。
Apple II HGR 是 non-linear addressing（非線性記憶體編址，
列號到記憶體位址的對應不是簡單乘法）：

```python
addr = $2000 + ((row & 7) << 10) + ((row >> 3) & 7) * 0x80
              + ((row >> 6) & 3) * 0x28
```

## 下一步

1. **寫 UCSD P-System reader（檔案系統讀取器）** — 約 100-200 行 Python
   - 讀 disk block 2（目錄區頭）
   - 走訪目錄項找到 `WIZBOOT.CODE` / `TITLE.DATA` 之類的檔案
   - 抽出 file bytes 成為一塊 flat blob（連續位元組塊）
2. **交叉比對：用 ApplePy 或 python-apple-disk**
   - 這些工具已支援 P-System
   - `pip install applepy` 或 git clone snafaru 的 tools
3. **用 AppleWin（Windows 上的 Apple II 模擬器）跑 boot 後 dump HGR**
   - 用 Linux Wine 跑 AppleWin
   - 截圖 boot 後 $2000-$3FFF 區域
   - 直接得到 8KB HGR framebuffer（跳過 LZ 重建步驟）

## 測試

雖然還原失敗，但工具鏈本身驗證過：

`tools/hgr_to_png.py` 跑合成資料（對角線斜紋）→ 正確產生紫綠藍橘的 NTSC artifact 色彩（見 `docs/hgr_test.png`）。

```bash
# 合成一張帶對角線的 HGR raw data
python3 -c "/* see commit history */"
# 用本專案的 HGR→PNG 轉換器渲染，--scale 2 是放大兩倍
python3 tools/hgr_to_png.py /tmp/test.hgr docs/hgr_test.png --scale 2
```

`tools/lz_decomp.py` 演算法依 Pascal 與 asm（組合語言）1:1 移植；token 邏輯已驗證（verify）。

## 替代方案

如果 P-System reader 太複雜，可考慮：

| 方法 | 工作量 | 產出 |
|------|--------|------|
| AppleWin emulator（Apple II 模擬器）dump | 2 小時 | 1 張靜態 PNG（沒動畫） |
| Mednafen DOS dump（DOS 多平台模擬器） | 2 小時 | PNG，但是 PC port 不是 Apple II 版 |
| 寫 P-System reader | 1-2 天 | 可完整 access 所有 disk 資料 |
| 用 snafaru 的 `latest-disk-images-System.Startup` 抽 | 1 天 | 一樣的 1981 美術 |

目前先把 LZ + HGR 工具留著，等 P-System reader 補上之後一鍵還原。
