//
// Created by Lazaro O'Farrill on 09/03/2023.
//

#include "zephyr/kernel.h"
#include "icm20948.h"
#include "icm20948-register-map.h"

T_IMU IMU = {
        .i2c_dev = NULL,
        .address = 0,
        .accX = 0,
        .accY = 0,
        .accZ = 0,
        .gyX = 0,
        .gyY = 0,
        .gyZ = 0
};


//Declarations
static int select_bank(uint8_t bank);

static int wake_up();

static int reset();

static int config_accel();

static int config_gyro();

static int config_mag();

static int self_test();

//Definitions

/**
 *
 * @param address Address of sensor
 * @retval 0   Success
 * @retval -EINVAL Invalid address value
 * @retval -ENXIO  i2c device not ready
 */
int init_imu(uint8_t address) {
    int err;
    if (address != 0x68 && address != 0x69) {
        return -EINVAL;
    }

    IMU.address = address;
    IMU.i2c_dev = DEVICE_DT_GET(DT_ALIAS(board_i2c));

    if (!device_is_ready(IMU.i2c_dev)) {
        return -ENXIO;
    }

    err = reset();
    if (err) {
        return err;
    }

    k_sleep(K_MSEC(1000));

    err = wake_up();
    if (err) {
        return err;
    }

    err = config_gyro();
    if (err) {
        return err;
    }

    err = config_accel();
    if (err) {
        return err;
    }

    err = self_test();
    if (err) {
        return err;
    }
}

int imu_read_sensors() {
    return 0;
}

int select_bank(uint8_t bank) {
    if (bank > 3) {
        return -EINVAL;
    }

    int err = 0;

    uint8_t regBankValue;

    err = i2c_reg_read_byte(IMU.i2c_dev, IMU.address, REG_BANK_SEL, &regBankValue);
    if (err) {
        return err;
    }

    regBankValue = regBankValue & 0xcf;
    regBankValue = regBankValue | (bank << 4);

    err = i2c_reg_write_byte(IMU.i2c_dev, IMU.address, REG_BANK_SEL, regBankValue);
    if (err) {
        return err;
    }


    return err;
}

int wake_up() {
    return 0;
}

int reset() {
    return 0;
}

int config_accel() {
    return 0;
}

int config_gyro() {
    return 0;
}

int config_mag() {
    return 0;
}

int self_test() {
    return 0;
}
