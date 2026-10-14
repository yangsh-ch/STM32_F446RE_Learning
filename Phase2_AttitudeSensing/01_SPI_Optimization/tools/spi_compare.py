"""
Compare a Logic 2 SPI decode export against the byte stream the firmware should send.

The expected stream is rebuilt from the repo source itself (font table and
oled_init() sequence are parsed from Core/Src/oled_ssd1306.cpp), so the reference
always matches the code that was actually flashed.

Logic 2 setup assumed:
  - SPI analyzer: MOSI=CH1, Clock=CH0, Enable=CH2, MISO=CH3 (DC line).
    DC is stable for the whole byte, so miso reads 0x00 for a command byte
    and 0xFF for a data byte.
  - Async Serial analyzer on CH4 (UART labels "[SPI] <Hz> Hz, cycle <N>").

Pipeline:
  1. expected_segment()  what each frequency segment should contain (from source)
  2. read_*_csv()        what Logic 2 actually decoded
  3. split_segments()    cut the decoded bytes into segments using the UART labels
  4. compare()           align expected vs actual byte by byte and classify each one
  5. report()            Markdown summary per segment

Usage:
  python spi_compare.py <spi.csv> <uart.csv> [--out report.md]
  python spi_compare.py --selftest
"""
import argparse
import csv
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
DRIVER_CPP = HERE.parent / "Core" / "Src" / "oled_ssd1306.cpp"

TEST_FREQS_HZ = [1000000, 4000000, 8000000, 16000000]  # must match 01_main.cpp


# ---------------------------------------------------------------- expected stream

def load_driver(path=DRIVER_CPP):
    """Return (font, init_cmds) parsed from the driver source.
    font[i] is the 5-byte glyph of ASCII 0x20 + i; init_cmds is every
    oled_write_command(0x..) in oled_init(), in call order."""
    src = path.read_text(encoding="utf-8")

    # Body of `static const uint8_t font5x7[][5] = { ... };`, one {..} row per glyph
    table = re.search(r"font5x7\[\]\[5\]\s*=\s*\{(.*?)\n\};", src, re.S).group(1)
    font = [[int(x, 16) for x in re.findall(r"0x[0-9A-Fa-f]{2}", row)]
            for row in re.findall(r"\{([^{}]*)\}", table)]
    assert all(len(g) == 5 for g in font), "font row parse error"

    body = re.search(r"void oled_init\(\)\s*\{(.*?)\n\}", src, re.S).group(1)
    init_cmds = [int(x, 16) for x in re.findall(r"oled_write_command\((0x[0-9A-Fa-f]{2})\)", body)]
    return font, init_cmds


class Stream:
    """Ordered list of (kind, value, where) with kind 'C' (command) or 'D' (data).
    `where` names the code that sends the byte, so a mismatch in the report
    points straight back to the source."""

    def __init__(self):
        self.items = []

    def cmd(self, values, where):   # bytes sent via oled_write_command() (DC low)
        self.items += [("C", v, where) for v in values]

    def data(self, values, where):  # bytes sent via oled_write_data() (DC high)
        self.items += [("D", v, where) for v in values]


def draw_string(s, text, col, page, font):
    # Mirrors oled_draw_string() / oled_draw_char() / oled_set_cursor()
    for ch in text:
        if col > 122:                       # same right-edge stop as the driver
            break
        c = ord(ch)
        if c < 0x20 or c > 0x7A:            # same clamp as oled_draw_char(): unsupported -> space
            c = 0x20
        where = f"'{chr(c)}' col {col} page {page}"
        s.cmd([0x21, col, 0x7F, 0x22, page, 0x07], f"cursor {where}")   # oled_set_cursor()
        s.data(font[c - 0x20] + [0x00], f"glyph {where}")                # 5 glyph cols + 1 blank
        col += 6


def expected_segment(hz, cycle, font):
    # Mirrors one iteration of the for-loop in 01_main.cpp
    s = Stream()
    s.cmd([0x21, 0x00, 0x7F, 0x22, 0x00, 0x07], "oled_clear cursor")
    s.data([0x00] * 1024, "oled_clear 0x00")                             # 128 cols x 8 pages
    draw_string(s, "SPI Freq Test", 0, 0, font)
    draw_string(s, f"Now: {hz // 1000000} MHz", 0, 2, font)
    draw_string(s, "Hello STM32!", 0, 4, font)
    draw_string(s, f"Cycle: {cycle}", 0, 6, font)
    return s.items


# ---------------------------------------------------------------- Logic 2 CSV

def _col(header, *keys):
    # Index of the first header cell containing any of `keys` (case-insensitive)
    for i, h in enumerate(header):
        name = h.strip().strip('"').lower()
        if any(k in name for k in keys):
            return i
    return None


def _hex(v):
    v = v.strip().strip('"')
    return int(v, 16) if v else None


# Logic 2 "Export to TXT/CSV" format (verified with captures/20261012_*.csv):
#   SPI : Time [s],Packet ID,MOSI,MISO          one row per decoded byte
#   UART: Time [s],Value,Parity Error,Framing Error
#         Value is the character itself; ',' is written as COMMA, CR/LF as \r \n

def read_spi_csv(path):
    """Return one dict per decoded byte: time, kind (C/D from the DC line), value."""
    rows = list(csv.reader(open(path, encoding="utf-8-sig", newline="")))
    h = rows[0]
    i_type, i_t = _col(h, "type"), _col(h, "time", "start")
    i_mosi, i_miso = _col(h, "mosi"), _col(h, "miso")
    if None in (i_t, i_mosi):
        sys.exit(f"Unexpected SPI CSV header: {h}")
    if i_miso is None:
        sys.exit("SPI CSV has no miso column: set SPI analyzer MISO = Channel 3 (DC) and re-export")
    out = []
    for r in rows[1:]:
        if i_type is not None and r[i_type].strip().strip('"') != "result":
            continue                                    # other export formats also list enable/disable rows
        miso = _hex(r[i_miso])
        kind = {0x00: "C", 0xFF: "D"}.get(miso, "?")    # "?" would mean DC changed mid-byte
        out.append({"t": float(r[i_t]), "kind": kind, "value": _hex(r[i_mosi]), "miso": miso})
    return out


UART_TOKENS = {"COMMA": ",", "\\r": "\r", "\\n": "\n", "": " "}


def read_uart_labels(path):
    """Rebuild the UART text and return every '[SPI] <Hz> Hz, cycle <N>' label
    with the timestamp of its first character."""
    rows = list(csv.reader(open(path, encoding="utf-8-sig", newline="")))
    h = rows[0]
    i_t, i_d = _col(h, "time", "start"), _col(h, "value", "data")
    text, times = "", []                                # times[k] = timestamp of text[k]
    for r in rows[1:]:
        d = r[i_d]
        if d in UART_TOKENS:
            ch = UART_TOKENS[d]
        elif d.startswith("0x") and len(d) == 4:        # hex display mode
            ch = chr(int(d, 16))
        else:
            ch = d[0]
        text += ch
        times.append(float(r[i_t]))
    labels = []
    for m in re.finditer(r"\[SPI\] (\d+) Hz, cycle (\d+)", text):
        labels.append({"hz": int(m.group(1)), "cycle": int(m.group(2)), "t": times[m.start()]})
    return labels


# ---------------------------------------------------------------- compare

def classify(e, a):
    """Status of one aligned (expected, actual) byte pair.
    shift1/shift0: decoder counted one extra clock at the start of the byte, so every
    bit moved one place right and the MSB was read as 1 (shift1) or 0 (shift0)."""
    ek, ev, _ = e
    if a["kind"] != ek:
        return "diff"
    av = a["value"]
    if av == ev:
        return "ok"
    if av == 0x80 | (ev >> 1):
        return "shift1"
    if av == ev >> 1:
        return "shift0"
    return "diff"


# Alignment costs: a mismatched byte must stay cheaper than a gap (missing/extra),
# otherwise the aligner would rather shift the whole tail than accept one bad byte.
COST = {"ok": 0, "shift1": 1, "shift0": 1, "diff": 2}
GAP = 3


def compare(actual, expected, band=40):
    """Banded global alignment (Needleman-Wunsch). Substitutions are cheaper than
    insert/delete, so a corrupted byte stays a substitution and only a truly
    missing/extra byte shifts the alignment.

    cost[i][j] = cheapest way to align expected[:i] with actual[:j].
    Only cells within `band` of the diagonal are computed: the two lengths differ
    by a few bytes at most, so the full n x m table is unnecessary."""
    n, m = len(expected), len(actual)
    lo_off, hi_off = min(0, m - n) - band, max(0, m - n) + band
    INF = float("inf")
    cost = [dict() for _ in range(n + 1)]
    back = [dict() for _ in range(n + 1)]               # which step reached each cell
    cost[0][0] = 0
    for i in range(n + 1):
        for j in range(max(0, i + lo_off), min(m, i + hi_off) + 1):
            if i == 0 and j == 0:
                continue
            best, step = INF, None
            # Pair expected[i-1] with actual[j-1]
            if i > 0 and j > 0 and (j - 1) in cost[i - 1]:
                c = cost[i - 1][j - 1] + COST[classify(expected[i - 1], actual[j - 1])]
                if c < best:
                    best, step = c, "sub"
            # expected[i-1] has no counterpart in the capture
            if i > 0 and j in cost[i - 1] and cost[i - 1][j] + GAP < best:
                best, step = cost[i - 1][j] + GAP, "missing"
            # actual[j-1] was not expected
            if j > 0 and (j - 1) in cost[i] and cost[i][j - 1] + GAP < best:
                best, step = cost[i][j - 1] + GAP, "extra"
            if step:
                cost[i][j], back[i][j] = best, step
    if m not in cost[n]:
        raise RuntimeError("alignment band too narrow")
    # Walk back from the bottom-right cell to recover the chosen alignment
    rows, i, j = [], n, m
    while i or j:
        step = back[i][j]
        if step == "sub":
            rows.append((classify(expected[i - 1], actual[j - 1]), expected[i - 1], actual[j - 1]))
            i, j = i - 1, j - 1
        elif step == "missing":
            rows.append(("missing", expected[i - 1], None))
            i -= 1
        else:
            rows.append(("extra", None, actual[j - 1]))
            j -= 1
    return rows[::-1]


def split_segments(spi, labels, font, init_cmds):
    """Each UART label is sent right AFTER its segment's SPI traffic.
    The 1MHz / cycle 0 segment also starts with oled_init() (board was just reset).
    The first segment of a capture is partial unless it is that post-reset one."""
    segments, start = [], None
    for lab in labels:
        # Bytes between the previous label and this one belong to this segment
        seg = [x for x in spi if (start is None or x["t"] > start) and x["t"] < lab["t"]]
        exp = expected_segment(lab["hz"], lab["cycle"], font)
        after_reset = lab["cycle"] == 0 and lab["hz"] == TEST_FREQS_HZ[0]
        if after_reset:
            pre = Stream()
            pre.cmd(init_cmds, "oled_init")
            exp = pre.items + exp
        lab = dict(lab, partial=(start is None and not after_reset))
        segments.append((lab, seg, exp))
        start = lab["t"]
    return segments


# ---------------------------------------------------------------- report

def fmt(v):
    return "--" if v is None else f"0x{v:02X}"


STATUSES = ("ok", "shift1", "shift0", "diff", "missing", "extra")


def _table(rows):
    # Markdown table of (status, expected, actual) rows
    out = ["| 時間 (s) | 狀態 | C/D 預期→實際 | 預期 | 實際 | 對應程式 |",
           "| --- | --- | --- | --- | --- | --- |"]
    for status, e, a in rows:
        t = f"{a['t']:.6f}" if a else "--"
        kd = f"{e[0] if e else '-'}→{a['kind'] if a else '-'}"
        out.append(f"| {t} | {status} | {kd} | {fmt(e[1] if e else None)} "
                   f"| {fmt(a['value'] if a else None)} | {e[2] if e else ''} |")
    return out


def report(segments, examples=8, max_rows=60):
    """Summary table (count per status per segment), then for each segment every
    unexplained byte (diff/missing/extra) and a few shift examples."""
    lines = ["狀態說明：ok=吻合；shift1=實際值 = 0x80 | (預期 >> 1)；shift0=實際值 = 預期 >> 1；"
             "diff=其他不符；missing=少了這個 byte；extra=多出來的 byte",
             "",
             "| 頻率段 | 預期 byte | 實際 byte | ok | shift1 | shift0 | diff | missing | extra |",
             "| --- | --- | --- | --- | --- | --- | --- | --- | --- |"]
    details = []
    for lab, seg, exp in segments:
        name = f"{lab['hz'] // 1000000}MHz (cycle {lab['cycle']})"
        if lab["partial"]:
            lines.append(f"| {name} | -- | {len(seg)} | 擷取開始前就在傳，不完整，略過 | | | | | |")
            continue
        rows = compare(seg, exp)
        cnt = {s: sum(1 for r in rows if r[0] == s) for s in STATUSES}
        lines.append(f"| {name} | {len(exp)} | {len(seg)} | "
                     + " | ".join(str(cnt[s]) for s in STATUSES[:]) + " |")
        shifted = [r for r in rows if r[0] in ("shift1", "shift0")]
        other = [r for r in rows if r[0] in ("diff", "missing", "extra")]
        details.append(f"\n### {name}\n")
        if other:
            details.append(f"diff / missing / extra（共 {len(other)} 筆，列前 {min(len(other), max_rows)} 筆）：\n")
            details += _table(other[:max_rows])
        else:
            details.append("沒有 diff / missing / extra。")
        if shifted:
            details.append(f"\nshift 範例（共 {len(shifted)} 筆，列前 {min(len(shifted), examples)} 筆）：\n")
            details += _table(shifted[:examples])
    return "\n".join(lines + details) + "\n"


# ---------------------------------------------------------------- self test

def selftest():
    """Build fake Logic 2 exports from the expected stream, inject known faults,
    and check the comparer reports exactly those faults."""
    import tempfile
    font, init_cmds = load_driver()

    # Spot checks against values read directly from the source
    assert init_cmds == [0xAE, 0x20, 0x00, 0x21, 0x00, 0x7F, 0x22, 0x00, 0x07,
                         0x8D, 0x14, 0x81, 0xFF, 0xA1, 0xC8, 0xA6, 0xAF], init_cmds
    assert font[ord("H") - 0x20] == [0x7F, 0x08, 0x08, 0x08, 0x7F]
    assert len(font) == 0x7A - 0x20 + 1

    tmp = Path(tempfile.mkdtemp())
    spi_csv, uart_csv = tmp / "spi.csv", tmp / "uart.csv"
    t, spi_rows, uart_rows = 0.0, [], []

    def emit(items, label, faults=None, drop=(), extra=()):
        # Append one segment's bytes (optionally corrupted / dropped / with an
        # extra byte) followed by its UART label, in Logic 2's export format
        nonlocal t
        for j, (k, v, _) in enumerate(items):
            if j in drop:
                continue
            if faults and j in faults:
                v = faults[j](v)
            spi_rows.append([f"{t:.9f}", "0", f"0x{v:02X}", "0x00" if k == "C" else "0xFF"])
            t += 2e-5
            if j in extra:
                spi_rows.append([f"{t:.9f}", "0", "0x55", "0xFF"])
                t += 2e-5
        for ch in label:
            tok = {",": "COMMA", "\r": "\\r", "\n": "\\n", " ": ""}.get(ch, ch)
            uart_rows.append([f"{t:.9f}", tok, "", ""])
            t += 8.7e-5
        t += 5.0

    # Tail of a segment from before the reset (capture started mid-transfer)
    emit(expected_segment(8000000, 14, font)[-300:], "[SPI] 8000000 Hz, cycle 14\r\n")

    # After reset: init + 1/4/8/16MHz, with one known fault of each kind injected
    seg1 = [("C", v, "") for v in init_cmds] + expected_segment(1000000, 0, font)
    s_idx = next(j for j, (k, v, _) in enumerate(seg1) if k == "D" and v == 0x46)  # 'S' col 0
    shift1 = lambda v: 0x80 | (v >> 1)
    emit(seg1, "[SPI] 1000000 Hz, cycle 0\r\n",
         faults={0: shift1, s_idx: lambda v: v | 0x80})                         # shift1 + diff
    emit(expected_segment(4000000, 0, font), "[SPI] 4000000 Hz, cycle 0\r\n")   # clean
    emit(expected_segment(8000000, 0, font), "[SPI] 8000000 Hz, cycle 0\r\n",
         drop={0}, extra={1500})                                                # missing + extra
    emit(expected_segment(16000000, 0, font), "[SPI] 16000000 Hz, cycle 0\r\n",
         faults={j: shift1 for j in range(6, 1030, 3)})                         # many 0x00 -> 0x80

    with open(spi_csv, "w", newline="") as f:
        csv.writer(f).writerows([["Time [s]", "Packet ID", "MOSI", "MISO"]] + spi_rows)
    with open(uart_csv, "w", newline="") as f:
        csv.writer(f).writerows([["Time [s]", "Value", "Parity Error", "Framing Error"]] + uart_rows)

    segs = split_segments(read_spi_csv(spi_csv), read_uart_labels(uart_csv), font, init_cmds)
    assert [s[0]["hz"] for s in segs] == [8000000] + TEST_FREQS_HZ
    assert [s[0]["partial"] for s in segs] == [True, False, False, False, False]
    got = []
    for lab, seg, exp in segs[1:]:
        rows = compare(seg, exp)
        got.append(tuple(sum(1 for r in rows if r[0] == s) for s in ("shift1", "diff", "missing", "extra")))
    want = [(1, 1, 0, 0), (0, 0, 0, 0), (0, 0, 1, 1), (len(range(6, 1030, 3)), 0, 0, 0)]
    assert got == want, got
    print("selftest OK: parsed", len(init_cmds), "init cmds,", len(font), "glyphs;",
          "injected faults detected per segment (shift1, diff, missing, extra):", got)


def main():
    sys.stdout.reconfigure(encoding="utf-8")   # Windows console defaults to cp950
    ap = argparse.ArgumentParser()
    ap.add_argument("spi_csv", nargs="?")
    ap.add_argument("uart_csv", nargs="?")
    ap.add_argument("--out")
    ap.add_argument("--selftest", action="store_true")
    args = ap.parse_args()

    if args.selftest:
        selftest()
        return
    if not (args.spi_csv and args.uart_csv):
        ap.error("need spi_csv and uart_csv (or --selftest)")

    font, init_cmds = load_driver()
    labels = read_uart_labels(args.uart_csv)
    if not labels:
        sys.exit("No '[SPI] <Hz> Hz, cycle <N>' label found in UART CSV")
    segs = split_segments(read_spi_csv(args.spi_csv), labels, font, init_cmds)
    text = report(segs)
    if args.out:
        Path(args.out).write_text(text, encoding="utf-8")
    print(text)


if __name__ == "__main__":
    main()
