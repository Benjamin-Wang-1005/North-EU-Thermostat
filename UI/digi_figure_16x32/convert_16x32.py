import pandas as pd
import os

def convert():
    output_h_file = "my_custom_digits_16x32.h"

    # 1. 抓取目前目錄下所有 .csv 檔案
    csv_files = [f for f in os.listdir('.') if f.lower().endswith('.csv')]

    # 2. 排序檔案 (確保 工作表1, 工作表2... 的順序是正確的)
    # Windows 的排序通常會把 10 排在 1 後面，這裡做一個簡單排序
    csv_files.sort()

    if not csv_files:
        print("錯誤：資料夾內找不到任何 .csv 檔案！")
        return

    print("--- 偵測到以下檔案 ---")
    for i, f_name in enumerate(csv_files):
        print(f"Index {i} -> {f_name}")
    print("---------------------\n")

    with open(output_h_file, 'w', encoding='utf-8') as f:
        f.write("// North EU Thermostat - Custom 16x32 Fonts\n")
        f.write("#ifndef __MY_CUSTOM_DIGITS_16X32_H__\n")
        f.write("#define __MY_CUSTOM_DIGITS_16X32_H__\n\n")

        # 這裡假設您的 CSV 剛好就是 10 個，對應數字 0-9
        for idx, file_name in enumerate(csv_files):
            if idx >= 10: break # 只處理前 10 個

            print(f"正在轉換: {file_name} 為數字 {idx}...")

            try:
                # 讀取 CSV (考慮到 Excel 轉出的 CSV 可能編碼不同，增加 encoding 參數)
                try:
                    df = pd.read_csv(file_name, header=None, encoding='utf-8').fillna('')
                except:
                    df = pd.read_csv(file_name, header=None, encoding='gbk').fillna('')

                # 擷取 32(高) x 16(寬) 區域 (16x32 = 16寬 x 32高)
                data = df.iloc[:32, :16].values

                f.write(f"// Digit {idx} (Source: {file_name})\n")
                f.write(f"static const unsigned char custom_16x32_{idx}[64] = {{\n    ")

                bytes_list = []
                for r in range(32):
                    for b_idx in range(2): # 16bit / 8 = 2 bytes
                        current_byte = 0
                        for bit in range(8):
                            col = b_idx * 8 + bit
                            if col < data.shape[1]:
                                cell = str(data[r][col])
                                # 只要格子裡有 '█' 或任何非空字元
                                if '█' in cell or (cell.strip() != ''):
                                    current_byte |= (0x80 >> bit)
                        bytes_list.append(f"0x{current_byte:02X}")

                # 每 12 個 Byte 換一行
                for i in range(0, len(bytes_list), 12):
                    f.write(",".join(bytes_list[i:i+12]))
                    if i + 12 < len(bytes_list):
                        f.write(",\n    ")

                f.write("\n};\n\n")
            except Exception as e:
                print(f"檔案 {file_name} 轉換出錯: {e}")

        f.write("#endif\n")

    print(f"\n轉換成功！產出檔案：{output_h_file}")

if __name__ == "__main__":
    convert()
