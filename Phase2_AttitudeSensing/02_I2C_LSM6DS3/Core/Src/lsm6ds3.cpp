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

uint8_t lsm6ds3_who_am_i() {
    uint8_t id = 0;
    lsm6ds3_read_reg(0x0F, &id, 1);
    return id;
}
