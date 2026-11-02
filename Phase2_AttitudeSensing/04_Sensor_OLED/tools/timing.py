"""2-04 loop timing from a Logic 2 raw CSV (Time, SCL, SDA, SCLK, CS, MOSI, DC).
A frame = one burst of I2C + SPI activity; frames are separated by idle gaps > 20 ms.
Per frame: I2C span (first..last SCL edge), I2C transactions (START count),
SPI span (first CS fall .. last CS rise), CS windows (bytes), command/data bytes,
and loop period (I2C start to next I2C start)."""
import csv, sys, statistics as st

rows = []
with open(sys.argv[1], newline="") as f:
    r = csv.reader(f); next(r)
    for t, scl, sda, sclk, cs, mosi, dc in r:
        rows.append((float(t), int(scl), int(sda), int(cs), int(dc)))

events = []          # (t, kind)  kind: 'scl', 'start', 'csf', 'csr'
p = rows[0]
for cur in rows[1:]:
    t, scl, sda, cs, dc = cur
    if scl != p[1]:
        events.append((t, "scl"))
    if sda != p[2] and sda == 0 and scl == 1 and p[1] == 1:
        events.append((t, "start"))          # SDA falls while SCL high
    if cs != p[3]:
        events.append((t, "csf" if cs == 0 else "csr", dc))
    p = cur

frames, cur = [], []
for e in events:
    if cur and e[0] - cur[-1][0] > 20e-3:
        frames.append(cur); cur = []
    cur.append(e)
if cur: frames.append(cur)

res = []
for fr in frames:
    scl = [e[0] for e in fr if e[1] == "scl"]
    csf = [e for e in fr if e[1] == "csf"]
    csr = [e[0] for e in fr if e[1] == "csr"]
    if not scl or not csf:
        continue
    res.append(dict(
        i2c0=scl[0], i2c1=scl[-1], starts=sum(1 for e in fr if e[1] == "start"),
        spi0=csf[0][0], spi1=csr[-1] if csr else None,
        nbytes=len(csf), ncmd=sum(1 for e in csf if e[2] == 0)))

full = [x for x in res if x["spi1"] and x["i2c0"] < x["spi0"]]
def show(name, xs, unit=1e3, u="ms"):
    xs = [x * unit for x in xs]
    print(f"{name:28s} n={len(xs):3d}  min {min(xs):8.3f}  median {st.median(xs):8.3f}  max {max(xs):8.3f} {u}")

print("frames:", len(res), "complete:", len(full))
show("I2C span", [x["i2c1"] - x["i2c0"] for x in full])
show("I2C end -> SPI start", [x["spi0"] - x["i2c1"] for x in full])
show("OLED update (SPI span)", [x["spi1"] - x["spi0"] for x in full])
show("frame busy (I2C0..SPI1)", [x["spi1"] - x["i2c0"] for x in full])
periods = [b["i2c0"] - a["i2c0"] for a, b in zip(full, full[1:])]
show("loop period", periods)
print("I2C STARTs per frame:", sorted(set(x["starts"] for x in full)))
print("SPI bytes per frame:", sorted(set(x["nbytes"] for x in full)), " cmd bytes:", sorted(set(x["ncmd"] for x in full)))
byte_period = [(x["spi1"] - x["spi0"]) / x["nbytes"] for x in full]
show("avg time per SPI byte", byte_period, 1e6, "us")
