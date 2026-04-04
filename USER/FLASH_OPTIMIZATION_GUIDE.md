# STM32 Flash 記憶體優化指南

## 問題
目前的程式碼中，Icon 和字型資料雖然使用了 `const` 宣告，但在 STM32 編譯環境中預設還是會佔用 RAM 空間。

## 解決方案

### 1. 修改標頭檔 (icons.h 和 arial_digits_generated.h)

在檔案開頭加入以下巨集定義：

```c
#ifndef FLASH_CONST
    #define FLASH_CONST const
#endif
```

然後把所有的資料宣告從：
```c
const unsigned char gImage_icon1[336] = { ... };
```

改成：
```c
static FLASH_CONST unsigned char gImage_icon1[336] = { ... };
```

### 2. 關鍵修改點

#### icons.h 修改：
- 所有 `const unsigned char gImage_iconX[]` → `static FLASH_CONST unsigned char gImage_iconX[]`
- `ICON_INFO` 結構體中的 `const unsigned char *data` → `FLASH_CONST unsigned char *data`
- `icon_table[]` 加上 `static FLASH_CONST`

#### arial_digits_generated.h 修改：
- 所有字型陣列加上 `static FLASH_CONST`
- 指標陣列 `arial_32x64_digits[]` 等也要加上 `static FLASH_CONST`

### 3. 編譯器設定 (如果上述方法無效)

如果編譯後仍然佔用 RAM，可能需要在編譯器設定中確認：

#### Keil MDK:
- 在 Options for Target → Target 中確認 Read/Only Memory Areas 設定正確
- 確保 .rodata 段被放在 Flash 區域

#### STM32CubeIDE / GCC:
- 檢查 linker script (.ld 檔案)，確保 `.rodata` 段在 Flash 區域：
```
.rodata :
{
    . = ALIGN(4);
    *(.rodata)
    *(.rodata*)
    . = ALIGN(4);
} >FLASH
```

### 4. 驗證方法

編譯後檢查 MAP 檔案，確認：
1. 字型資料位址在 Flash 區域 (通常是 0x08000000 開始)
2. RAM 使用量減少

或者使用以下程式碼在執行時檢查：
```c
// 檢查資料位址是否在 Flash 區域 (STM32F103 Flash 從 0x08000000 開始)
if ((uint32_t)arial_32x64_0 >= 0x08000000 && (uint32_t)arial_32x64_0 < 0x08020000) {
    // 資料在 Flash 中
}
```

## 預期效果

以目前的字型資料估算：
- 32x64 字型 (10個數字): ~2.5KB
- 16x32 字型 (10個數字): ~640B
- 12x24 字型 (10個數字): ~480B
- Icons: ~10KB+

**總計可節省約 15-20KB RAM**

## 注意事項

1. 使用 `static` 限制檔案內可見性，避免命名衝突
2. 資料在 Flash 中，**不可修改** (嘗試寫入會造成 HardFault)
3. 讀取速度比 RAM 稍慢，但對於 LCD 顯示來說影響不大
