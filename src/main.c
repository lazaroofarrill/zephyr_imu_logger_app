#include <zephyr/kernel.h>
#include "zephyr/drivers/i2c.h"
#include "i2c_utils.h"

#define I2C_P1 DT_NODELABEL(i2c1)
#define ICM_ADDR 0x68

//Registers
//Bank_0
#define WHO_AM_I 0x00
#define USER_CTRL 0x03
#define LP_CONFIG 0x05
#define PWR_MGMT_1 0x06
#define PWR_MGMT_2 0x07
#define INT_PIN_CFG 0x0F
#define INT_ENABLE 0x10
#define INT_ENABLE_1 0x11
#define INT_ENABLE_2 0x12
#define INT_ENABLE_3 0x13
#define I2C_MST_STATUS 0x17
#define INT_STATUS 0x19
#define INT_STATUS_1 0x1A
#define INT_STATUS_2 0x1B
#define INT_STATUS_3 0x1C
#define DELAY_TIMEH 0x28
#define DELAY_TIMEL 0x29
#define ACCEL_XOUT_H 0x2D
#define ACCEL_XOUT_L 0x2E
#define ACCEL_YOUT_H 0x2F
#define ACCEL_YOUT_L 0x30
#define ACCEL_ZOUT_H 0x31
#define ACCEL_ZOUT_L 0x32
#define GYRO_XOUT_H 0x33
#define GYRO_XOUT_L 0x34
#define GYRO_YOUT_H 0x35
#define GYRO_YOUT_L 0x36
#define GYRO_ZOUT_H 0x37
#define GYRO_ZOUT_L 0x38
#define TEMP_OUT_H 0x39
#define TEMP_OUT_L 0x3A
#define EXT_SLV_SENS_DATA_00 0x3B
#define EXT_SLV_SENS_DATA_01 0x3C
#define EXT_SLV_SENS_DATA_02 0x3D
#define EXT_SLV_SENS_DATA_03 0x3E
#define EXT_SLV_SENS_DATA_04 0x3F
#define EXT_SLV_SENS_DATA_05 0x40
#define EXT_SLV_SENS_DATA_06 0x41
#define EXT_SLV_SENS_DATA_07 0x42
#define EXT_SLV_SENS_DATA_08 0x43
#define EXT_SLV_SENS_DATA_09 0x44
#define EXT_SLV_SENS_DATA_10 0x45
#define EXT_SLV_SENS_DATA_11 0x46
#define EXT_SLV_SENS_DATA_12 0x47
#define EXT_SLV_SENS_DATA_13 0x48
#define EXT_SLV_SENS_DATA_14 0x49
#define EXT_SLV_SENS_DATA_15 0x4A
#define EXT_SLV_SENS_DATA_16 0x4B
#define EXT_SLV_SENS_DATA_17 0x4C
#define EXT_SLV_SENS_DATA_18 0x4D
#define EXT_SLV_SENS_DATA_19 0x4E
#define EXT_SLV_SENS_DATA_20 0x4F
#define EXT_SLV_SENS_DATA_21 0x50
#define EXT_SLV_SENS_DATA_22 0x51
#define EXT_SLV_SENS_DATA_23 0x52

//User Bank 1
#define SELF_TEST_X_GYRO 0x02
#define SELF_TEST_Y_GYRO 0x03
#define SELF_TEST_Z_GYRO 0x04
#define SELF_TEST_X_ACCEL 0x0E
#define SELF_TEST_Y_ACCEL 0x0F
#define SELF_TEST_Z_ACCEL 0x10


//User Bank 2
#define GYRO_SMPLRT_DIV 0x00
#define GYRO_CONFIG_1 0x01
#define GYRO_CONFIG_2 0x02
#define ACCEL_CONFIG 0x14
#define ACCEL_CONFIG_2 0x15


//Common
#define FIFO_EN_1 0x66
#define REG_BANK_SEL 0x7F

uint8_t bit_mask(uint8_t value, uint8_t mask, int *err) {
    uint8_t maskStart = mask >> 4;
    uint8_t maskEnd = mask & 0xf;
    if (maskStart > 7 || maskEnd > 7 || maskStart < maskEnd) {
        *err = -EINVAL;
        return 0;
    }

    uint8_t maskBitMap = 0x00;
    for (int i = maskEnd; i <= maskStart; ++i) {
        maskBitMap = maskBitMap | (1 << maskEnd);
    }
    return value & maskBitMap;
}

int select_bank(const struct device *const i2c_dev, uint16_t address, uint8_t bank) {
    int err;

    uint8_t regBankValue = 0x00;

    err = i2c_reg_read_byte(i2c_dev, address, REG_BANK_SEL, &regBankValue);
    if (err < 0) {
        return -EIO;
    }

    regBankValue = bit_mask(regBankValue, 0x76, &err)
                   | bit_mask(regBankValue, 0x30, &err);
    regBankValue = (bank << 3) | regBankValue;


    err = i2c_reg_write_byte(i2c_dev, address, REG_BANK_SEL, regBankValue);
    if (err < 0) {
        return -EIO;
    }

    return 0;
}

int imu_read_sensor(const struct device *const i2c_dev, uint16_t address) {
    int err;

    const uint8_t readings[12] = {0};

    err = select_bank(i2c_dev, address, 0);
    if (err < 0) {
        printk("Error switching bank\n");
        return -EIO;
    }

    err = i2c_burst_read(i2c_dev,
                         address,
                         ACCEL_XOUT_H,
                         (uint8_t *) readings,
                         12);
    if (err < 0) {
        printk("Error reading accel data\n");
        return -EIO;
    }

    uint16_t accX = (int16_t) ((((int16_t) readings[0]) << 8) | readings[1]);
    uint16_t accY = (int16_t) (((int16_t) readings[2] << 8) | readings[3]);
    uint16_t accZ = (int16_t) (((int16_t) readings[4] << 8) | readings[5]);
    uint16_t gyX = (int16_t) (((int16_t) readings[6] << 8) | readings[7]);
    uint16_t gyY = (int16_t) (((int16_t) readings[8] << 8) | readings[9]);
    uint16_t gyZ = (int16_t) (((int16_t) readings[10] << 8) | readings[11]);

    printk("%20d%20d%20d%20d%20d%20d\n", accX, accY, accZ, gyX, gyY, gyZ);

    return 0;
}

void main(void) {
    const struct device *const i2c_dev = DEVICE_DT_GET(I2C_P1);
    printk("\n");

    if (!device_is_ready(i2c_dev)) {
        printk("Could not get i2c device\n");
        return;
    }

    printk("Hello World! %s\n", CONFIG_BOARD);
    printk("Scanning i2c:\n");

    for (uint8_t i = 3; i < 0x78; i++) {
        int result = i2c_ping(i2c_dev, i);

        if (i % 16 == 0)
            printk("\n%.2x:", i);

        if (result == 0) {
            printk(" %.2x", i);
        } else
            printk(" --");
    }

    while (1) {
        imu_read_sensor(i2c_dev, ICM_ADDR);
        k_sleep(K_MSEC(100));
    }
}
