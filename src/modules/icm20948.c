//
// Created by Lazaro O'Farrill on 09/03/2023.
//

#include <zephyr/logging/log.h>
#include "zephyr/kernel.h"
#include "icm20948.h"
#include "icm20948-register-map.h"

LOG_MODULE_DECLARE(imu_logger);


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
        LOG_ERR("Error: %s:%d. Error code(%d)", __FILE__, __LINE__, err);
        return -EINVAL;
    }

    IMU.address = address;
    IMU.i2c_dev = DEVICE_DT_GET(DT_ALIAS(board_i2c));

    if (!device_is_ready(IMU.i2c_dev)) {
        LOG_ERR("Error: %s:%d. Error code(%d)", __FILE__, __LINE__, err);
        return -ENXIO;
    }

    err = reset();
    if (err) {
        LOG_ERR("Error: %s:%d. Error code(%d)", __FILE__, __LINE__, err);
        return err;
    }

    k_sleep(K_MSEC(1));

    err = wake_up();
    if (err) {
        LOG_ERR("Error: %s:%d. Error code(%d)", __FILE__, __LINE__, err);
        return err;
    }


    err = config_gyro();
    if (err) {
        LOG_ERR("Error: %s:%d. Error code(%d)", __FILE__, __LINE__, err);
        return err;
    }

    err = config_accel();
    if (err) {
        LOG_ERR("Error: %s:%d. Error code(%d)", __FILE__, __LINE__, err);
        return err;
    }

    err = self_test();
    if (err) {
        LOG_ERR("Error: %s:%d. Error code(%d)", __FILE__, __LINE__, err);
        return err;
    }
    return err;
}

int imu_read_sensors() {

    uint8_t readings[12] = {0};

    int err = select_bank(0);
    if (err) {
        return err;
    }

    err = i2c_burst_read(IMU.i2c_dev, IMU.address, ACCEL_XOUT_H, (uint8_t *) readings, 12);
    if (err) {
        return err;
    }

    IMU.accX = (((int16_t) readings[0]) << 8) | readings[1];
    IMU.accY = (((int16_t) readings[2]) << 8) | readings[3];
    IMU.accZ = (((int16_t) readings[4]) << 8) | readings[5];
    IMU.gyX = (((int16_t) readings[6]) << 8) | readings[7];
    IMU.gyY = (((int16_t) readings[8]) << 8) | readings[9];
    IMU.gyZ = (((int16_t) readings[10]) << 8) | readings[11];


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
    int err = 0;

    uint8_t powerManagementReg = 0x00;

    err = i2c_reg_read_byte(IMU.i2c_dev, IMU.address, PWR_MGMT_1, &powerManagementReg);
    if (err) {
        return err;
    }

    powerManagementReg = powerManagementReg & 0xbf;

    err = i2c_reg_write_byte(IMU.i2c_dev, IMU.address, PWR_MGMT_1, powerManagementReg);
    if (err) {
        return err;
    }

    return err;
}

int reset() {
    int err = i2c_reg_write_byte(IMU.i2c_dev, IMU.address, PWR_MGMT_1, 0x80);
    if (err) {
        return err;
    }

    k_sleep(K_MSEC(1));

    return 0;
}

typedef enum {
    ACCEL_FS_SEL_2G = 0 << 1,
    ACCEL_FS_SEL_4G = 1 << 1,
    ACCEL_FS_SEL_8G = 2 << 1,
    ACCEL_FS_SEL_16G = 3 << 1,
} accel_fs_sel;

typedef enum {
    ACCEL_FCHOICE_DISABLE,
    ACCEL_FCHOICE_ENABLE
} accel_fchoice;

typedef enum {
    ACCEL_DLPFCFG_0 = 0 << 3,
    ACCEL_DLPFCFG_1 = 1 << 3,
    ACCEL_DLPFCFG_2 = 2 << 3,
    ACCEL_DLPFCFG_3 = 3 << 3,
    ACCEL_DLPFCFG_4 = 4 << 3,
    ACCEL_DLPFCFG_5 = 5 << 3,
    ACCEL_DLPFCFG_6 = 6 << 3,
    ACCEL_DLPFCFG_7 = 7 << 3,
} accel_dlpfcfg;

int config_accel() {
    int err;

    uint8_t accelConfigReg;

    err = i2c_reg_read_byte(IMU.i2c_dev, IMU.address, ACCEL_CONFIG, &accelConfigReg);
    if (err) {
        return err;
    }

    accelConfigReg = accelConfigReg & ((1 << 7) | (1 << 6));
    accelConfigReg = accelConfigReg | ACCEL_DLPFCFG_0 | ACCEL_FS_SEL_2G | ACCEL_FCHOICE_ENABLE;

    err = i2c_reg_write_byte(IMU.i2c_dev, IMU.address, ACCEL_CONFIG, accelConfigReg);

    return err;
}

typedef enum {
    GYRO_F_DISABLE, //Disable low pass filter
    GYRO_F_ENABLE //Enable low pass filter
} gyro_fchoice;

typedef enum {
    GYRO_FS_250 = 0 << 1,
    GYRO_FS_500 = 1 << 1,
    GYRO_FS_1000 = 2 << 1,
    GYRO_FS_2000 = 3 << 1,
} gyro_fs_sel;

typedef enum {
    GYRO_DLPFCFG_0 = 0 << 3,
    GYRO_DLPFCFG_1 = 1 << 3,
    GYRO_DLPFCFG_2 = 2 << 3,
    GYRO_DLPFCFG_3 = 2 << 3,
    GYRO_DLPFCFG_4 = 4 << 3,
    GYRO_DLPFCFG_5 = 5 << 3,
    GYRO_DLPFCFG_6 = 6 << 3,
    GYRO_DLPFCFG_7 = 7 << 3,
} gyro_dlpfcfg;

int config_gyro() {
    uint8_t gyroConfig1Reg;

    int err = select_bank(2);
    if (err) {
        return err;
    }

    err = i2c_reg_read_byte(IMU.i2c_dev, IMU.address, GYRO_CONFIG_1, &gyroConfig1Reg);
    if (err) {
        return err;
    }

    gyroConfig1Reg = gyroConfig1Reg & ((1 << 7) | (1 << 6));
    gyroConfig1Reg = gyroConfig1Reg | GYRO_DLPFCFG_0 | GYRO_FS_250 | GYRO_F_ENABLE;

    err = i2c_reg_write_byte(IMU.i2c_dev, IMU.address, GYRO_CONFIG_1, gyroConfig1Reg);
    if (err) {
        return err;
    }

    return 0;
}

int config_mag() {
    return 0;
}

typedef enum {
    AX_ST_EN_REG_DISABLE = 0 << 4,
    AX_ST_EN_REG_ENABLE = 1 << 4,
} ax_st_en_reg;

typedef enum {
    AY_ST_EN_REG_DISABLE = 0 << 3,
    AY_ST_EN_REG_ENABLE = 1 << 3,
} ay_st_en_reg;

typedef enum {
    AZ_ST_EN_REG_DISABLE = 0 << 2,
    AZ_ST_EN_REG_ENABLE = 1 << 2,
} az_st_en_reg;

typedef enum {
    DEC3_CFG_1_4,
    DEC3_CFG_8,
    DEC3_CFG_16,
    DEC3_CFG_32,
} dec3_cfg;

int self_test() {
    int err;
    uint8_t accelConfig2Reg;

    err = select_bank(2);
    if (err) {
        return err;
    }

    err = i2c_reg_read_byte(IMU.i2c_dev, IMU.address, ACCEL_CONFIG_2, &accelConfig2Reg);
    if (err) {
        return err;
    }

    accelConfig2Reg = accelConfig2Reg & ((1 << 7) | (1 << 6) | (1 << 5));
    accelConfig2Reg = accelConfig2Reg | AX_ST_EN_REG_ENABLE | AY_ST_EN_REG_ENABLE | AZ_ST_EN_REG_ENABLE;
    err = i2c_reg_write_byte(IMU.i2c_dev, IMU.address, ACCEL_CONFIG_2, accelConfig2Reg);
    if (err) {
        return err;
    }

    err = imu_read_sensors();
    if (err) {
        return err;
    }

    int16_t testAxis[3] = {IMU.accX, IMU.accY, IMU.accZ};

    accelConfig2Reg = accelConfig2Reg & ((1 << 7) | (1 << 6));

    err = i2c_reg_write_byte(IMU.i2c_dev, IMU.address, ACCEL_CONFIG_2, accelConfig2Reg);
    if (err) {
        return err;
    }

    err = imu_read_sensors();
    if (err) {
        return err;
    }

    err = select_bank(1);
    if (err) {
        return err;
    }

    uint8_t selfTestGyroRef[3], selfTestAccelRef[3];
    err = i2c_burst_read(IMU.i2c_dev, IMU.address, SELF_TEST_X_GYRO, selfTestGyroRef, 3);
    if (err) {
        return err;
    }

    err = i2c_burst_read(IMU.i2c_dev, IMU.address, SELF_TEST_X_ACCEL, selfTestAccelRef, 3);
    if (err) {
        return err;
    }

    const char axis[3] = {'X', 'Y', 'Z'};
    const uint16_t testDisabledAxis[3] = {IMU.accX, IMU.accY, IMU.accZ};

    for (int i = 0; i < 3; i++) {
        printk("\nSelf test response %c: %d --- Expected %d",
               axis[i], testAxis[i] - testDisabledAxis[i],
               selfTestAccelRef[i]);
    }


    return 0;
}
