# 專案編輯注意事項

## 1. Edit 工具縮排問題

### 問題
C 原始碼檔案使用 **Tab** 字元縮排。在使用 Edit 工具時，如果 `old_string` / `new_string` 中的縮排字元（Tab / Space）與原始檔案不一致，Edit 工具會回報「String to replace not found」。

### 避開方法
- 使用 Edit 前，**務必先使用 Read 工具讀取目標區域**，確認檔案使用的是 Tab 還是 Space。
- 從 Read 結果直接複製貼上到 Edit 的 `old_string`，確保縮排完全一致。
- **不要手動輸入縮排**，手動輸入時很容易把 Tab 打成 Space 或數量不一致。

---

## 2. 換行與轉義序列問題

### 問題
專案原始碼在 Windows 環境下使用 **CRLF (`\r\n`)** 換行。當 Edit 的 `old_string` 或 `new_string` 包含字面上的轉義序列（如 `\r\n`、`\t`）時，這些字元可能被錯誤解析為實際的換行或 Tab，導致：
- 單行字串被拆成多行（例如 `LOGE("Flash write fail!\r\n");` 變成兩行）。
- C 編譯器報錯：字符串字面量跨行、macro invocation 未正確結束、大括號不匹配等連鎖錯誤。

### 避開方法
- **優先從 Read 結果複製貼上**：不要手動輸入包含 `\r\n` 或 `\t` 的字串。
- 如果必須手動輸入，確保 `\r\n` 最終在檔案中呈現為六個字元（`\`、`r`、`\`、`n`），而不是實際的換行。
- 編輯後，使用 `cat -A` 或 Read 工具再次確認該行沒有被拆成多行。
- **如果 Edit 多次失敗**：改用 Bash + `sed` 或 Python 腳本處理，避免在 Edit 字串中硬拗轉義序列。
