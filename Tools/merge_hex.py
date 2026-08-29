import os

# === Configuration ===
BOOT_BIN     = "IAP_Bootloader.bin"
BOOT_HEX     = "IAP_Bootloader.hex"
APP_BIN      = "TOUCH.bin"
APP_HEX      = "TOUCH.hex"
OUT_DIR      = os.path.join(os.getcwd(), "Production")
OUT_BIN      = "Merged_Firmware.bin"
OUT_HEX      = "Merged_Firmware.hex"

FLASH_BASE   = 0x08000000
BOOT_SIZE    = 0x00004000        # 16KB reserved for bootloader

script_dir = os.path.dirname(os.path.abspath(__file__))
os.makedirs(OUT_DIR, exist_ok=True)

def merge_bin():
    boot = open(os.path.join(script_dir, BOOT_BIN), "rb").read()
    app  = open(os.path.join(script_dir, APP_BIN),  "rb").read()

    boot = boot + b'\xFF' * (BOOT_SIZE - len(boot))
    out = boot + app
    open(os.path.join(OUT_DIR, OUT_BIN), "wb").write(out)
    print(f"[BIN] Boot={len(boot)} App={len(app)} Total={len(out)}")

def merge_hex():
    """Concatenate boot hex + app hex (remove EOF from boot)."""
    boot = open(os.path.join(script_dir, BOOT_HEX), "r").read()
    app  = open(os.path.join(script_dir, APP_HEX),  "r").read()

    lines = boot.strip().split('\n')
    if lines[-1].strip() == ':00000001FF':
        lines = lines[:-1]

    merged = '\n'.join(lines) + '\n' + app.strip() + '\n'
    open(os.path.join(OUT_DIR, OUT_HEX), "w").write(merged)
    print(f"[HEX] Concatenated: boot hex + app hex")

if __name__ == "__main__":
    merge_bin()
    merge_hex()
