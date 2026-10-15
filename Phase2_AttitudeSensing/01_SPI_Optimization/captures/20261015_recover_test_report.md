狀態說明：ok=吻合；shift1=實際值 = 0x80 | (預期 >> 1)；shift0=實際值 = 預期 >> 1；diff=其他不符；missing=少了這個 byte；extra=多出來的 byte

| 頻率段 | 預期 byte | 實際 byte | ok | shift1 | shift0 | diff | missing | extra |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1MHz (cycle 19) | -- | 1557 | 擷取開始前就在傳，不完整，略過 | | | | | |
| 1MHz (cycle 0) | 1563 | 1561 | 798 | 763 | 0 | 0 | 2 | 0 |
| 4MHz (cycle 0) | 1546 | 1545 | 784 | 761 | 0 | 0 | 1 | 0 |
| 8MHz (cycle 0) | 1546 | 1545 | 761 | 784 | 0 | 0 | 1 | 0 |
| 16MHz (cycle 0) | 1558 | 1557 | 783 | 774 | 0 | 0 | 1 | 0 |
| 1MHz (cycle 1) | 1546 | 1545 | 790 | 755 | 0 | 0 | 1 | 0 |

### 1MHz (cycle 0)

diff / missing / extra（共 2 筆，列前 2 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| -- | missing | C→- | 0xAE | -- | oled_init |
| -- | missing | C→- | 0x21 | -- | oled_clear cursor |

shift 範例（共 763 筆，列前 8 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| 6.004714 | shift1 | C→C | 0x21 | 0x90 | oled_init |
| 6.004774 | shift1 | C→C | 0x22 | 0x91 | oled_init |
| 6.004814 | shift1 | C→C | 0x07 | 0x83 | oled_init |
| 6.004874 | shift1 | C→C | 0x81 | 0xC0 | oled_init |
| 6.005020 | shift1 | C→C | 0x00 | 0x80 | oled_clear cursor |
| 6.005100 | shift1 | C→C | 0x07 | 0x83 | oled_clear cursor |
| 6.005120 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 6.005160 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |

### 4MHz (cycle 0)

diff / missing / extra（共 1 筆，列前 1 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| -- | missing | C→- | 0x21 | -- | oled_clear cursor |

shift 範例（共 761 筆，列前 8 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| 11.036010 | shift1 | C→C | 0x00 | 0x80 | oled_clear cursor |
| 11.036022 | shift1 | C→C | 0x07 | 0x83 | oled_clear cursor |
| 11.036070 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 11.036094 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 11.036118 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 11.036142 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 11.036166 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 11.036190 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |

### 8MHz (cycle 0)

diff / missing / extra（共 1 筆，列前 1 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| -- | missing | C→- | 0x21 | -- | oled_clear cursor |

shift 範例（共 784 筆，列前 8 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| 16.054274 | shift1 | C→C | 0x00 | 0x80 | oled_clear cursor |
| 16.054318 | shift1 | C→C | 0x07 | 0x83 | oled_clear cursor |
| 16.054341 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 16.054352 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 16.054363 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 16.054374 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 16.054386 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 16.054398 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |

### 16MHz (cycle 0)

diff / missing / extra（共 1 筆，列前 1 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| -- | missing | C→- | 0x21 | -- | oled_clear cursor |

shift 範例（共 774 筆，列前 8 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| 21.072600 | shift1 | C→C | 0x22 | 0x91 | oled_clear cursor |
| 21.072622 | shift1 | C→C | 0x07 | 0x83 | oled_clear cursor |
| 21.072633 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 21.072644 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 21.072655 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 21.072667 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 21.072679 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 21.072691 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |

### 1MHz (cycle 1)

diff / missing / extra（共 1 筆，列前 1 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| -- | missing | C→- | 0x21 | -- | oled_clear cursor |

shift 範例（共 755 筆，列前 8 筆）：

| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |
| --- | --- | --- | --- | --- | --- |
| 26.090929 | shift1 | C→C | 0x22 | 0x91 | oled_clear cursor |
| 26.090949 | shift1 | C→C | 0x00 | 0x80 | oled_clear cursor |
| 26.091009 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 26.091049 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 26.091089 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 26.091129 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 26.091169 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
| 26.091209 | shift1 | D→D | 0x00 | 0x80 | oled_clear 0x00 |
