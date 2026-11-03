# -*- coding: utf-8 -*-
"""
convert.py - 把 16x16 圖示的點陣 CSV 轉成 C 陣列 (只用 Python 標準函式庫)

CSV 格式: 每格一個點。空白或 0 = 不亮；1、█ 或其他字元 = 亮。
預設行為與舊版相同: 找出圖案的外框，把圖案置中放進 16x16 畫布。
輸出格式: static const uint8_t NAME[32]，MSB first，每列 2 bytes，
          符合 GUI_DrawMonoIcon16x16() 的格式。

用法:
  python convert.py ON_Icon_16x16.csv                       -> 陣列名稱 = ON_Icon_16x16
  python convert.py ON_Icon_16x16.csv -n ON_Icon_Setting_16x16
  python convert.py ON_Icon_16x16.csv -o out.txt            -> 同時寫到 out.txt (CRLF)
  python convert.py ON_Icon_16x16.csv --no-center           -> 不置中，照 CSV 左上角原位轉換
  python convert.py *_Icon_16x16.csv                        -> 一次轉多個檔
  加 --no-preview 可關掉預覽圖 (預覽印在 stderr，不會混進 Table)
  -W / -H 可改畫布寬高 (預設 16x16)
"""
import argparse
import csv
import glob
import os
import re
import sys


def read_rows(path):
    for enc in ("utf-8-sig", "cp950", "gbk"):
        try:
            with open(path, newline="", encoding=enc) as f:
                return list(csv.reader(f))
        except UnicodeDecodeError:
            continue
    sys.exit("錯誤: 無法辨識 %s 的文字編碼" % path)


def is_on(cell):
    c = cell.strip()
    return c != "" and c != "0"


def build_grid(path, w, h, center):
    rows = read_rows(path)
    pts = [(r, c) for r, row in enumerate(rows) for c, cell in enumerate(row) if is_on(cell)]
    grid = [[0] * w for _ in range(h)]
    if not pts:
        sys.stderr.write("警告: %s 裡沒有任何亮點\n" % path)
        return grid, (0, 0)

    min_r = min(p[0] for p in pts); max_r = max(p[0] for p in pts)
    min_c = min(p[1] for p in pts); max_c = max(p[1] for p in pts)
    cw, ch = max_c - min_c + 1, max_r - min_r + 1

    if center:
        if cw > w or ch > h:
            sys.stderr.write("警告: %s 圖案 %dx%d 大於畫布 %dx%d，超出部分會被裁掉\n"
                             % (path, cw, ch, w, h))
        ro = max(0, (h - ch) // 2) - min_r
        co = max(0, (w - cw) // 2) - min_c
    else:
        if max_c >= w or max_r >= h:
            sys.stderr.write("警告: %s 有點超出 %dx%d 範圍，已忽略\n" % (path, w, h))
        ro = co = 0

    for r, c in pts:
        rr, cc = r + ro, c + co
        if 0 <= rr < h and 0 <= cc < w:
            grid[rr][cc] = 1
    return grid, (cw, ch)


def to_bytes(grid, w):
    out = []
    for row in grid:
        for b in range(0, w, 8):
            v = 0
            for i in range(8):
                if b + i < w and row[b + i]:
                    v |= 0x80 >> i
            out.append(v)
    return out


def format_table(name, path, data, content, w, h):
    bpr = (w + 7) // 8
    lines = ["// Generated from %s" % os.path.basename(path),
             "// Content size: %dx%d, Canvas size: %dx%d" % (content[0], content[1], w, h),
             "static const uint8_t %s[%d] = {" % (name, len(data))]
    for i in range(0, len(data), bpr):
        hex_row = ", ".join("0x%02X" % v for v in data[i:i + bpr])
        lines.append("    " + hex_row + ("," if i + bpr < len(data) else ""))
    lines.append("};")
    return "\n".join(lines)


def default_name(path):
    name = re.sub(r"\W", "_", os.path.splitext(os.path.basename(path))[0])
    return name if not name[:1].isdigit() else "_" + name


def main():
    ap = argparse.ArgumentParser(description="16x16 圖示 CSV 轉 C Table")
    ap.add_argument("csv", nargs="+", help="CSV 檔 (可用萬用字元)")
    ap.add_argument("-n", "--name", help="陣列名稱 (只轉一個檔時可用，預設用檔名)")
    ap.add_argument("-o", "--output", help="另外寫入這個檔案 (CRLF)")
    ap.add_argument("-W", "--width", type=int, default=16, help="畫布寬 (預設 16)")
    ap.add_argument("-H", "--height", type=int, default=16, help="畫布高 (預設 16)")
    ap.add_argument("--no-center", action="store_true", help="不要自動置中")
    ap.add_argument("--no-preview", action="store_true", help="不要印預覽圖")
    args = ap.parse_args()

    files = []
    for p in args.csv:
        hits = glob.glob(p)
        files.extend(sorted(hits) if hits else [p])
    if args.name and len(files) > 1:
        sys.exit("錯誤: -n 只能用在單一檔案")

    tables = []
    for path in files:
        if not os.path.isfile(path):
            sys.exit("錯誤: 找不到檔案 %s" % path)
        grid, content = build_grid(path, args.width, args.height, not args.no_center)
        name = args.name or default_name(path)
        if not args.no_preview:
            sys.stderr.write("--- %s 預覽 (%s) ---\n" % (name, path))
            for row in grid:
                sys.stderr.write("".join("#" if b else "." for b in row) + "\n")
            sys.stderr.write("\n")
        tables.append(format_table(name, path, to_bytes(grid, args.width),
                                   content, args.width, args.height))

    out = "\n\n".join(tables) + "\n"
    sys.stdout.write(out)
    if args.output:
        with open(args.output, "w", encoding="ascii", newline="\r\n") as f:
            f.write(out)
        sys.stderr.write("已寫入 %s\n" % args.output)


if __name__ == "__main__":
    main()
