//
// Created by Lazaro O'Farrill on 20/03/2023.
//

#ifndef IMU_LOGGER_ICM20948_H
#define IMU_LOGGER_ICM20948_H

#include "stdint.h"
#include "zephyr/drivers/i2c.h"

int init_imu(uint8_t address);

int imu_read_sensors();

typedef struct T_IMU {
    struct device *i2c_dev;
    uint8_t address;
    int16_t accX;
    int16_t accY;
    int16_t accZ;
    int16_t gyX;
    int16_t gyY;
    int16_t gyZ;
} T_IMU;

extern T_IMU IMU;

#endif //IMU_LOGGER_ICM20948_H
