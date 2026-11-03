import csv
import math

def convert_csv_to_c_array(file_path, array_name, target_w=16, target_h=16):
    # Use Unicode escape for the block character to avoid encoding issues in script
    target_char = "\u2588" 
    rows = []
    try:
        # We still open the CSV as utf-8 because the CSV itself contains the block char
        with open(file_path, 'r', encoding='utf-8') as f:
            reader = csv.reader(f)
            for row in reader:
                # Check if block char is in cell
                binary_row = [1 if target_char in cell else 0 for cell in row]
                if binary_row:
                    rows.append(binary_row)
    except FileNotFoundError:
        print("Error: File not found")
        return

    input_h = len(rows)
    input_w = max(len(r) for r in rows) if rows else 0
    grid = [[0 for _ in range(target_w)] for _ in range(target_h)]

    # Center the icon
    v_offset = (target_h - input_h) // 2
    h_offset = (target_w - input_w) // 2

    for r in range(min(input_h, target_h)):
        for c in range(min(input_w, target_w)):
            if (r + v_offset < target_h) and (c + h_offset < target_w):
                grid[r + v_offset][c + h_offset] = rows[r][c]

    bytes_per_row = math.ceil(target_w / 8)
    bytes_list = []
    for r in range(target_h):
        for b in range(0, target_w, 8):
            byte_val = 0
            for i in range(8):
                if b + i < target_w:
                    if grid[r][b + i]:
                        byte_val |= (1 << (7 - i))
            bytes_list.append(byte_val)

    # Output C code
    print(f"// {array_name} - {target_w}x{target_h} bitmap")
    print(f"static const uint8_t {array_name}[{len(bytes_list)}] = {{")
    for i in range(0, len(bytes_list), bytes_per_row):
        row_data = bytes_list[i : i + bytes_per_row]
        hex_row = ", ".join([f"0x{b:02X}" for b in row_data])
        suffix = "," if i + bytes_per_row < len(bytes_list) else ""
        print(f"    {hex_row}{suffix}")
    print("};")

if __name__ == "__main__":
    convert_csv_to_c_array('ON_Icon_32x32.csv', 'ON_Icon_32x32', 32, 32)