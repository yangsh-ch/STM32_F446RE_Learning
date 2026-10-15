狀態說明：ok=吻合；shift1=實際值 = 0x80 | (預期 >> 1)；shift0=實際值 = 預期 >> 1；diff=其他不符；missing=少了這個 byte；extra=多出來的 byte

| 頻率段 | 預期 byte | 實際 byte | ok | shift1 | shift0 | diff | missing | extra |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 8MHz (cycle 3) | -- | 1546 | 擷取開始前就在傳，不完整，略過 | | | | | |
| 1MHz (cycle 0) | 1563 | 1563 | 554 | 1009 | 0 | 0 | 0 | 0 |
| 4MHz (cycle 0) | 1546 | 1546 | 511 | 1035 | 0 | 0 | 0 | 0 |
| 8MHz (cycle 0) | 1546 | 1546 | 537 | 1009 | 0 | 0 | 0 | 0 |
| 16MHz (cycle 0) | 1558 | 1558 | 499 | 1059 | 0 | 0 | 0 | 0 |
| 1MHz (cycle 1) | 1546 | 1546 | 557 | 989 | 0 | 0 | 0 | 0 |

### 1MHz (cycle 0)

沒有 diff / missing / extra。

shift 範例（共 1009 筆，列前 8 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| 7.243906 | shift1 | C→C | 0xAE | 0xD7 | oled_init |
| 7.243966 | shift1 | C→C | 0x21 | 0x90 | oled_init |
| 7.243986 | shift1 | C→C | 0x00 | 0x80 | oled_init |
| 7.244006 | shift1 | C→C | 0x7F | 0xBF | oled_init |
| 7.244026 | shift1 | C→C | 0x22 | 0x91 | oled_init |
| 7.244126 | shift1 | C→C | 0x81 | 0xC0 | oled_init |
| 7.244291 | shift1 | C→C | 0x00 | 0x80 | oled_clear cursor |
| 7.244311 | shift1 | C→C | 0x7F | 0xBF | oled_clear cursor |

### 4MHz (cycle 0)

沒有 diff / missing / extra。

shift 範例（共 1035 筆，列前 8 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| 12.275251 | shift1 | C→C | 0x7F | 0xBF | oled_clear cursor |
| 12.275275 | shift1 | C→C | 0x00 | 0x80 | oled_clear cursor |
| 12.275287 | shift1 | C→C | 0x07 | 0x83 | oled_clear cursor |
| 12.275323 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 12.275335 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 12.275371 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 12.275407 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 12.275515 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |

### 8MHz (cycle 0)

沒有 diff / missing / extra。

shift 範例（共 1009 筆，列前 8 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| 17.293529 | shift1 | C→C | 0x21 | 0x90 | oled_clear cursor |
| 17.293562 | shift1 | C→C | 0x22 | 0x91 | oled_clear cursor |
| 17.293595 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 17.293606 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 17.293617 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 17.293639 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 17.293650 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 17.293672 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |

### 16MHz (cycle 0)

沒有 diff / missing / extra。

shift 範例（共 1059 筆，列前 8 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| 22.310831 | shift1 | C→C | 0x21 | 0x90 | oled_clear cursor |
| 22.310853 | shift1 | C→C | 0x7F | 0xBF | oled_clear cursor |
| 22.310864 | shift1 | C→C | 0x22 | 0x91 | oled_clear cursor |
| 22.310897 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 22.310908 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 22.310919 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 22.310930 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 22.310941 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |

### 1MHz (cycle 1)

沒有 diff / missing / extra。

shift 範例（共 989 筆，列前 8 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| 27.328162 | shift1 | C→C | 0x00 | 0x80 | oled_clear cursor |
| 27.328182 | shift1 | C→C | 0x7F | 0xBF | oled_clear cursor |
| 27.328222 | shift1 | C→C | 0x00 | 0x80 | oled_clear cursor |
| 27.328242 | shift1 | C→C | 0x07 | 0x83 | oled_clear cursor |
| 27.328262 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 27.328282 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 27.328302 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 27.328322 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
