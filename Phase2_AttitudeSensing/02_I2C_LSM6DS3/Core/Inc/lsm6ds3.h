#ifndef LSM6DS3_H
#define LSM6DS3_H

#include "mbed.h"

static const uint8_t LSM6DS3_ADDR7 = 0x6B;   // SA0 measured high; confirm with the Address Scan result

void    lsm6ds3_init(I2C &bus);              // driver uses the bus owned by main (shared with the scan)
bool    lsm6ds3_read_reg(uint8_t reg, uint8_t *buf, int len);
bool    lsm6ds3_write_reg(uint8_t reg, uint8_t val);
uint8_t lsm6ds3_who_am_i();

#endif
