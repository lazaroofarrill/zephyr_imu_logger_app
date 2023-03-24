//
// Created by Lazaro O'Farrill on 21/03/2023.
//

#ifndef IMU_LOGGER_SDCARD_H
#define IMU_LOGGER_SDCARD_H

#include "zephyr/device.h"
#include "zephyr/storage/disk_access.h"
#include "zephyr/fs/fs.h"
#include "ff.h"

static FATFS fatFs;

static struct fs_mount_t mp = {
        .type = FS_FATFS,
        .fs_data = &fatFs,
        .mnt_point = "/SD:"
};

#endif //IMU_LOGGER_SDCARD_H
