//
// Created by Lazaro O'Farrill on 17/03/2023.
//

#ifndef IMU_LOGGER_I2C_UTILS_H
#define IMU_LOGGER_I2C_UTILS_H

#include "stdint.h"
#include "zephyr/drivers/i2c.h"


int i2c_ping(const struct device *device, uint16_t addr);

#endif //IMU_LOGGER_I2C_UTILS_H
