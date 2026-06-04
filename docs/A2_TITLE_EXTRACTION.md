# Apple II Wizardry I — 1981 Title Screen Extraction

把原版 Apple II 的 280×192 HGR title screen 還原到 PNG。

## 目前進度

✅ **完成**：
- `tools/lz_decomp.py` — Python port of LZDECOMP.TEXT (Apple II 6502 assembly)
- `tools/hgr_to_png.py` — Apple II HGR framebuffer → 280×192 PNG，含 NTSC artifact 色彩
- `tools/extract_a2_title.py` — DSK 暴力掃描器

⚠️ **遇到瓶頸**：DSK 是 **UCSD Pascal P-System** 格式，
title screen 的 LZ blob 不在簡單的 byte offset，需要先解 P-System 目錄。

暴力掃 440 個 candidate offsets，最高 score 0.744 但渲染出來都是雜訊。

## 還原原理

snafaru repo 內 `TITLELOA.TEXT` 顯示：

```pascal
WTBUFF : ARRAY[ 0..4607] OF INTEGER;  (* 9 BLOCKS == 18 SECTORS *)

BEGIN
  LZDECOMP( WTBUFF[ 0], WTBUFF[ 8]);   (* 主 title *)
  ...
  LZDECOMP( WTBUFF[ 0], WTBUFF[ 0]);   (* 動畫 frame 1 *)
END;
```

- **WTBUFF 是 9216 bytes（18 sectors）**從磁碟 block 載入
- **`LZDECOMP(WTBUFF, offset)`** 在 buffer 內偏移處讀 LZ 流，直接寫進 HGR ($2000-$3FFF)

所以正確做法：
1. 從 P-System filesystem 找出 title 相關 data file
2. 該 file 內容就是 WTBUFF[0..4607]
3. 依 `LZDECOMP(WTBUFF[0], WTBUFF[N])` 的偏移 N 解壓

## LZDECOMP 演算法（已實作）

從 6502 asm 移植：

```
Token at current pointer:
  byte < 0xC0  → row index; read 1-byte mask + up to 5 literal bytes
  0xC0..0xFC   → (col_offset = byte - 0xC0, row, value) tuple
  0xFD         → XOR-flash effect (animation, ignored for static)
  0xFE         → scroll-left effect (animation, ignored)
  0xFF / other → end
```

關鍵點：寫入位置由 row 透過 HGR_LO[row] / HGR_HI[row] 查表得到。
Apple II HGR 是 non-linear addressing：

```python
addr = $2000 + ((row & 7) << 10) + ((row >> 3) & 7) * 0x80
              + ((row >> 6) & 3) * 0x28
```

## 下一步

1. **寫 UCSD P-System reader** — 約 100-200 lines Python
   - Read disk block 2 (directory header)
   - Walk dir entries to find files like `WIZBOOT.CODE`, `TITLE.DATA`
   - Extract file bytes to a flat blob
2. **Cross-reference 用 ApplePy or python-apple-disk**
   - 這些工具已支援 P-System
   - `pip install applepy` 或 git clone snafaru's tools
3. **用 AppleWin 跑 boot 後 dump HGR**
   - 用 Linux Wine 跑 AppleWin (Windows emulator)
   - 截圖 boot 後 $2000-$3FFF 區域
   - 直接得到 8KB HGR framebuffer（跳過 LZ 重建）

## 測試

雖然還原失敗，但工具鏈本身驗證過：

`tools/hgr_to_png.py` 跑合成資料（對角線斜紋）→ 正確產生紫綠藍橘的 NTSC artifact 色彩（見 `docs/hgr_test.png`）。

```bash
# 合成 HGR with diagonal
python3 -c "/* see commit history */"
python3 tools/hgr_to_png.py /tmp/test.hgr docs/hgr_test.png --scale 2
```

`tools/lz_decomp.py` 演算法依 Pascal asm 1:1 移植；token 邏輯已 verify。

## 替代方案

如果 P-System reader 太複雜，可考慮：

| 方法 | 工作量 | 產出 |
|------|--------|------|
| AppleWin emulator dump | 2 小時 | 1 張靜態 PNG（沒動畫） |
| Mednafen DOS dump | 2 小時 | PNG，但是 PC port 不是 Apple II 版 |
| 寫 P-System reader | 1-2 天 | 完整 access 所有 disk 資料 |
| 用 snafaru 的 `latest-disk-images-System.Startup` 抽 | 1 天 | 一樣的 1981 art |

目前先把 LZ + HGR 工具留著，等 P-System reader 補上之後一鍵還原。
