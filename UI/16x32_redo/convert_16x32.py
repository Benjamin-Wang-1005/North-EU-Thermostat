# -*- coding: utf-8 -*-
"""
convert_16x32.py - 把 16x32 點陣 CSV 轉成 C 陣列 (只用 Python 標準函式庫，不需要 pandas)

CSV 格式: 32 列 (高) x 16 欄 (寬)。
  空白或 0       -> 不亮
  1、█ 或其他字元 -> 亮

輸出格式與 USER/arial_digits_flash.h 的 arial_16x32_N 相同:
  每列 2 bytes，由上到下；每個 byte 最高位 (0x80) 是最左邊的點。
  每行 12 bytes，共 64 bytes。

用法:
  python convert_16x32.py 16x32_3.csv              -> 印出 arial_16x32_3 的 Table
  python convert_16x32.py 16x32_3.csv -o out.txt   -> 同時寫到 out.txt
  python convert_16x32.py 16x32_3.csv -n my_font_3 -> 自訂陣列名稱
  python convert_16x32.py 16x32_*.csv              -> 一次轉多個檔
  加 --no-preview 可關掉預覽圖 (預覽印在 stderr，不會混進 Table)
"""
import argparse
import csv
import glob
import os
import re
import sys

WIDTH = 16
HEIGHT = 32
BYTES_PER_ROW = WIDTH // 8
BYTES_PER_LINE = 12


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


def convert(path):
    rows = read_rows(path)
    # 去掉檔尾的空列 (Excel 有時會多存)
    while rows and not any(c.strip() for c in rows[-1]) and len(rows) > HEIGHT:
        rows.pop()

    if len(rows) != HEIGHT:
        sys.stderr.write("警告: %s 有 %d 列，預期 %d 列 (不足補空白，多的忽略)\n"
                         % (path, len(rows), HEIGHT))
    for r, row in enumerate(rows[:HEIGHT]):
        extra = [c for c in row[WIDTH:] if is_on(c)]
        if extra:
            sys.stderr.write("警告: %s 第 %d 列在第 16 欄之後還有點，已忽略\n" % (path, r + 1))

    bitmap = []
    for r in range(HEIGHT):
        row = rows[r] if r < len(rows) else []
        bitmap.append([is_on(row[c]) if c < len(row) else False for c in range(WIDTH)])

    data = []
    for r in range(HEIGHT):
        for b in range(BYTES_PER_ROW):
            v = 0
            for bit in range(8):
                if bitmap[r][b * 8 + bit]:
                    v |= 0x80 >> bit
            data.append(v)
    return bitmap, data


def default_name(path):
    m = re.search(r"(\d+)\D*$", os.path.splitext(os.path.basename(path))[0])
    if m:
        return "arial_16x32_%s" % m.group(1)
    return "arial_16x32_x"


def format_table(name, path, data):
    m = re.search(r"(\d+)$", name)
    label = m.group(1) if m else name
    lines = ["// Digit %s (Source: %s)" % (label, os.path.basename(path)),
             "static const unsigned char %s[%d] = {" % (name, len(data))]
    body = []
    for i in range(0, len(data), BYTES_PER_LINE):
        body.append("    " + ",".join("0x%02X" % v for v in data[i:i + BYTES_PER_LINE]))
    lines.append(",\n".join(body))
    lines.append("};")
    return "\n".join(lines)


def main():
    ap = argparse.ArgumentParser(description="16x32 點陣 CSV 轉 C Table")
    ap.add_argument("csv", nargs="+", help="CSV 檔 (可用 16x32_*.csv)")
    ap.add_argument("-o", "--output", help="另外寫入這個檔案 (CRLF)")
    ap.add_argument("-n", "--name", help="陣列名稱 (只轉一個檔時可用)")
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
        bitmap, data = convert(path)
        name = args.name or default_name(path)
        if not args.no_preview:
            sys.stderr.write("--- %s 預覽 (%s) ---\n" % (name, path))
            for row in bitmap:
                sys.stderr.write("".join("#" if b else "." for b in row) + "\n")
            sys.stderr.write("\n")
        tables.append(format_table(name, path, data))

    out = "\n\n".join(tables) + "\n"
    sys.stdout.write(out)
    if args.output:
        with open(args.output, "w", encoding="ascii", newline="\r\n") as f:
            f.write(out)
        sys.stderr.write("已寫入 %s\n" % args.output)


if __name__ == "__main__":
    main()
