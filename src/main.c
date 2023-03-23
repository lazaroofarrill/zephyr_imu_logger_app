#include <zephyr/kernel.h>
#include "i2c_utils.h"
#include "icm20948.h"
#include "zephyr/logging/log.h"
#include "sdcard.h"
#include "stdio.h"
#include "zephyr/sys/util.h"

LOG_MODULE_REGISTER(imu_logger, LOG_LEVEL_INF);

void main(void) {
    int err;

    const struct device *const i2c_dev = DEVICE_DT_GET(DT_ALIAS(board_i2c));
    printk("\n");

    if (!device_is_ready(i2c_dev)) {
        printk("Could not get i2c device\n");
        return;
    }

    printk("Using board %s\n", CONFIG_BOARD);
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

    //Starting imu
    err = init_imu(0x68);
    if (err) {
        LOG_ERR("\nError initializing IMU (0x%x)\n", err);
    }

    printk("\n");
    //Starting SD card
    static const char *disk_pdrv = "SD";

    err = disk_access_init(disk_pdrv);
    if (err) {
        LOG_ERR("\nStorage init ERROR! (%d)\n", err);
        return;
    }

    err = fs_mount(&mp);
    if (err != FR_OK) {
        LOG_ERR("Error mounting disk.");
        return;
    }

    printk("\n");

    struct fs_file_t readsFile;

    fs_file_t_init(&readsFile);

    const char *readsFileName = "/SD:/reads.csv";

    fs_open(&readsFile, readsFileName, (FS_O_CREATE | FS_O_APPEND));
    fs_close(&readsFile);

    char buffer[255] = "";


    while (1) {
        imu_read_sensors();
        sprintf((char *) buffer, "%d,%d,%d\n", IMU.accX, IMU.accY, IMU.accZ);
        utf8_trunc(buffer);

        printk("%s", buffer);

        fs_open(&readsFile, readsFileName, (FS_O_APPEND | FS_O_WRITE));

        fs_write(&readsFile, buffer, strlen(buffer));

        fs_close(&readsFile);

        k_sleep(K_USEC(2500));
    }

}
