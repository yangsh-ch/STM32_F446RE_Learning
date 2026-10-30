狀態說明：ok=吻合；shift1=實際值 = 0x80 | (預期 >> 1)；shift0=實際值 = 預期 >> 1；diff=其他不符；missing=少了這個 byte；extra=多出來的 byte

| 頻率段 | 預期 byte | 實際 byte | ok | shift1 | shift0 | diff | missing | extra |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 8MHz (cycle 1) | -- | 1546 | 擷取開始前就在傳，不完整，略過 | | | | | |
| 1MHz (cycle 0) | 1563 | 1563 | 486 | 1077 | 0 | 0 | 0 | 0 |
| 4MHz (cycle 0) | 1546 | 1546 | 477 | 1069 | 0 | 0 | 0 | 0 |
| 8MHz (cycle 0) | 1546 | 1546 | 569 | 977 | 0 | 0 | 0 | 0 |
| 16MHz (cycle 0) | 1558 | 1558 | 507 | 1051 | 0 | 0 | 0 | 0 |
| 1MHz (cycle 1) | 1546 | 1546 | 483 | 1063 | 0 | 0 | 0 | 0 |
| 4MHz (cycle 1) | 1546 | 1546 | 463 | 1083 | 0 | 0 | 0 | 0 |

### 1MHz (cycle 0)

沒有 diff / missing / extra。

shift 範例（共 1077 筆，列前 8 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| 4.370520 | shift1 | C→C | 0x00 | 0x80 | oled_init |
| 4.370540 | shift1 | C→C | 0x21 | 0x90 | oled_init |
| 4.370580 | shift1 | C→C | 0x7F | 0xBF | oled_init |
| 4.370620 | shift1 | C→C | 0x00 | 0x80 | oled_init |
| 4.370640 | shift1 | C→C | 0x07 | 0x83 | oled_init |
| 4.370660 | shift1 | C→C | 0x8D | 0xC6 | oled_init |
| 4.370700 | shift1 | C→C | 0x81 | 0xC0 | oled_init |
| 4.370865 | shift1 | C→C | 0x00 | 0x80 | oled_clear cursor |

### 4MHz (cycle 0)

沒有 diff / missing / extra。

shift 範例（共 1069 筆，列前 8 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| 9.401808 | shift1 | C→C | 0x00 | 0x80 | oled_clear cursor |
| 9.401820 | shift1 | C→C | 0x7F | 0xBF | oled_clear cursor |
| 9.401844 | shift1 | C→C | 0x00 | 0x80 | oled_clear cursor |
| 9.401856 | shift1 | C→C | 0x07 | 0x83 | oled_clear cursor |
| 9.401880 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 9.402036 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 9.402060 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 9.402072 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |

### 8MHz (cycle 0)

沒有 diff / missing / extra。

shift 範例（共 977 筆，列前 8 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| 14.420095 | shift1 | C→C | 0x21 | 0x90 | oled_clear cursor |
| 14.420128 | shift1 | C→C | 0x22 | 0x91 | oled_clear cursor |
| 14.420161 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 14.420172 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 14.420183 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 14.420194 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 14.420216 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 14.420227 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |

### 16MHz (cycle 0)

沒有 diff / missing / extra。

shift 範例（共 1051 筆，列前 8 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| 19.437405 | shift1 | C→C | 0x00 | 0x80 | oled_clear cursor |
| 19.437416 | shift1 | C→C | 0x7F | 0xBF | oled_clear cursor |
| 19.437427 | shift1 | C→C | 0x22 | 0x91 | oled_clear cursor |
| 19.437449 | shift1 | C→C | 0x07 | 0x83 | oled_clear cursor |
| 19.437460 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 19.437471 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 19.437482 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 19.437493 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |

### 1MHz (cycle 1)

沒有 diff / missing / extra。

shift 範例（共 1063 筆，列前 8 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| 24.454702 | shift1 | C→C | 0x21 | 0x90 | oled_clear cursor |
| 24.454722 | shift1 | C→C | 0x00 | 0x80 | oled_clear cursor |
| 24.454762 | shift1 | C→C | 0x22 | 0x91 | oled_clear cursor |
| 24.454822 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 24.454842 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 24.454862 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 24.454882 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 24.454902 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |

### 4MHz (cycle 1)

沒有 diff / missing / extra。

shift 範例（共 1083 筆，列前 8 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| 29.485996 | shift1 | C→C | 0x21 | 0x90 | oled_clear cursor |
| 29.486008 | shift1 | C→C | 0x00 | 0x80 | oled_clear cursor |
| 29.486020 | shift1 | C→C | 0x7F | 0xBF | oled_clear cursor |
| 29.486044 | shift1 | C→C | 0x00 | 0x80 | oled_clear cursor |
| 29.486056 | shift1 | C→C | 0x07 | 0x83 | oled_clear cursor |
| 29.486080 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 29.486092 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 29.486128 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
