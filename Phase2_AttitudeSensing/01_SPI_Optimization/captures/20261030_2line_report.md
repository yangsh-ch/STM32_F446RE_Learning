狀態說明：ok=吻合；shift1=實際值 = 0x80 | (預期 >> 1)；shift0=實際值 = 預期 >> 1；diff=其他不符；missing=少了這個 byte；extra=多出來的 byte

| 頻率段 | 預期 byte | 實際 byte | ok | shift1 | shift0 | diff | missing | extra |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 8MHz (cycle 9) | -- | 1546 | 擷取開始前就在傳，不完整，略過 | | | | | |
| 1MHz (cycle 0) | 1563 | 1563 | 1563 | 0 | 0 | 0 | 0 | 0 |
| 4MHz (cycle 0) | 1546 | 1546 | 1546 | 0 | 0 | 0 | 0 | 0 |
| 8MHz (cycle 0) | 1546 | 1546 | 1546 | 0 | 0 | 0 | 0 | 0 |
| 16MHz (cycle 0) | 1558 | 1558 | 1558 | 0 | 0 | 0 | 0 | 0 |
| 1MHz (cycle 1) | 1546 | 1546 | 1546 | 0 | 0 | 0 | 0 | 0 |

### 1MHz (cycle 0)

沒有 diff / missing / extra。

### 4MHz (cycle 0)

沒有 diff / missing / extra。

### 8MHz (cycle 0)

沒有 diff / missing / extra。

### 16MHz (cycle 0)

沒有 diff / missing / extra。

### 1MHz (cycle 1)

沒有 diff / missing / extra。
