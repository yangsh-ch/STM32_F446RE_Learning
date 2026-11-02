#ifndef LSM6DS3_H
#define LSM6DS3_H

#include "mbed.h"

static const uint8_t LSM6DS3_ADDR7 = 0x6B;   // SA0 measured high; confirmed by the Phase 2-02 address scan

// Register addresses (datasheet Rev 10, section 9)
static const uint8_t LSM6DS3_CTRL1_XL   = 0x10;
static const uint8_t LSM6DS3_CTRL2_G    = 0x11;
static const uint8_t LSM6DS3_CTRL3_C    = 0x12;
static const uint8_t LSM6DS3_STATUS_REG = 0x1E;
static const uint8_t LSM6DS3_OUTX_L_G   = 0x22;   // gyro X/Y/Z then accel X/Y/Z: 12 bytes up to OUTZ_H_XL (0x2D)

struct Lsm6ds3Raw {
    int16_t gx, gy, gz;   // angular rate, raw LSB
    int16_t ax, ay, az;   // linear acceleration, raw LSB
};

void lsm6ds3_init(I2C &bus);                 // driver uses the I2C object owned by main
bool lsm6ds3_read_reg(uint8_t reg, uint8_t *buf, int len);
bool lsm6ds3_write_reg(uint8_t reg, uint8_t val);
bool lsm6ds3_configure();                    // 104Hz, +-2g, +-250dps, BDU; true if every register reads back
bool lsm6ds3_data_ready();                   // STATUS_REG: XLDA and GDA both set
bool lsm6ds3_read_raw(Lsm6ds3Raw *out);      // one 12-byte burst read starting at OUTX_L_G

#endif
