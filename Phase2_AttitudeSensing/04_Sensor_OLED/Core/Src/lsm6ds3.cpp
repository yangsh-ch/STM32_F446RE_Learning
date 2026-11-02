#include "lsm6ds3.h"

static I2C *i2c = nullptr;

void lsm6ds3_init(I2C &bus) {
    i2c = &bus;
}

bool lsm6ds3_read_reg(uint8_t reg, uint8_t *buf, int len) {
    char r = reg;
    if (i2c->write(LSM6DS3_ADDR7 << 1, &r, 1, true) != 0) return false;     // set register pointer, no STOP
    return i2c->read(LSM6DS3_ADDR7 << 1, (char *)buf, len) == 0;            // Sr, read len bytes, NACK on the last one
}

bool lsm6ds3_write_reg(uint8_t reg, uint8_t val) {
    char tx[2] = { (char)reg, (char)val };
    return i2c->write(LSM6DS3_ADDR7 << 1, tx, 2) == 0;                      // S -> 0xD6 -> A -> reg -> A -> val -> A -> P
}

bool lsm6ds3_configure() {
    static const struct { uint8_t reg, val; } cfg[] = {
        { LSM6DS3_CTRL3_C,  0x44 },   // BDU=1 (no half-updated samples), IF_INC=1 (auto-increment, default)
        { LSM6DS3_CTRL1_XL, 0x40 },   // ODR_XL=0100 (104Hz), FS_XL=00 (+-2g), BW_XL=00 (ignored: bandwidth follows ODR)
        { LSM6DS3_CTRL2_G,  0x40 },   // ODR_G=0100 (104Hz), FS_G=00 (250dps), FS_125=0
    };
    for (const auto &c : cfg) {
        uint8_t readback = 0;
        if (!lsm6ds3_write_reg(c.reg, c.val)) return false;
        if (!lsm6ds3_read_reg(c.reg, &readback, 1) || readback != c.val) return false;
    }
    return true;
}

bool lsm6ds3_data_ready() {
    uint8_t status = 0;
    if (!lsm6ds3_read_reg(LSM6DS3_STATUS_REG, &status, 1)) return false;
    return (status & 0x03) == 0x03;   // bit0 XLDA, bit1 GDA
}

bool lsm6ds3_read_raw(Lsm6ds3Raw *out) {
    uint8_t b[12];
    if (!lsm6ds3_read_reg(LSM6DS3_OUTX_L_G, b, sizeof(b))) return false;
    // Each axis is a 16-bit two's complement value, low byte at the lower address
    out->gx = (int16_t)(b[1]  << 8 | b[0]);
    out->gy = (int16_t)(b[3]  << 8 | b[2]);
    out->gz = (int16_t)(b[5]  << 8 | b[4]);
    out->ax = (int16_t)(b[7]  << 8 | b[6]);
    out->ay = (int16_t)(b[9]  << 8 | b[8]);
    out->az = (int16_t)(b[11] << 8 | b[10]);
    return true;
}
