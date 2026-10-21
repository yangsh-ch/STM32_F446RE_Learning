#include "mbed.h"
#include "lsm6ds3.h"

// I2C1: mbed takes the pins in (SDA, SCL) order
static I2C i2c(PB_9, PB_8);

static const uint8_t REG_WHO_AM_I = 0x0F;

// Address byte only, then check ACK: S -> addr+W -> ACK? -> P
static bool i2c_probe(uint8_t addr7) {
    i2c.start();
    int ack = i2c.write(addr7 << 1);   // single-byte overload: 1 = ACK, 0 = NACK, 2 = timeout
    i2c.stop();
    return ack == 1;
}

int main() {
    i2c.frequency(100000);             // 100kHz; step 4: change to 400000 and run again
    lsm6ds3_init(i2c);

    // Step 1: Address Scan
    printf("\r\n[I2C] scan 0x08-0x77\r\n");
    for (uint8_t a = 0x08; a <= 0x77; a++) {
        if (i2c_probe(a)) {
            printf("  found 0x%02X (wire W=0x%02X R=0x%02X)\r\n", a, a << 1, (a << 1) | 1);
        }
    }
    // The last probe (0x77) is NACKed and leaves the AF flag set; mbed's byte-level
    // stop() doesn't clear it, so the next full write() would abort right after START.
    I2C1->SR1 &= ~I2C_SR1_AF;

    // Step 3: WHO_AM_I: S -> 0xD6 -> A -> 0x0F -> A -> Sr -> 0xD7 -> A -> id -> NA -> P
    char reg = REG_WHO_AM_I, id = 0;
    int w = i2c.write(LSM6DS3_ADDR7 << 1, &reg, 1, true);   // true = no STOP, next transfer starts with Sr
    int r = i2c.read(LSM6DS3_ADDR7 << 1, &id, 1);           // mbed sets the R/W bit to 1 itself
    printf("[I2C] WHO_AM_I = 0x%02X (write=%d, read=%d)\r\n", (uint8_t)id, w, r);

    // Step 5: write then read back CTRL1_XL (0x10), datasheet 9.12 / Table 45-47
    const uint8_t REG_CTRL1_XL = 0x10;
    const uint8_t test_val = 0x10;                 // ODR_XL=0001 (12.5Hz), FS_XL=00 (+-2g), BW_XL=00
    uint8_t before = 0, after = 0;
    bool ok = lsm6ds3_read_reg(REG_CTRL1_XL, &before, 1)
           && lsm6ds3_write_reg(REG_CTRL1_XL, test_val)
           && lsm6ds3_read_reg(REG_CTRL1_XL, &after, 1);
    printf("[I2C] CTRL1_XL before=0x%02X write=0x%02X readback=0x%02X -> %s\r\n",
           before, test_val, after, (ok && after == test_val) ? "MATCH" : "MISMATCH");
    lsm6ds3_write_reg(REG_CTRL1_XL, before);       // restore: RESET only restarts the MCU, the sensor keeps its registers

    while (1) {
        ThisThread::sleep_for(1s);
    }
}
