//
// Created by Lazaro O'Farrill on 17/03/2023.
//

#include "i2c_utils.h"
#include "zephyr/drivers/i2c.h"

int i2c_ping(const struct device *dev, uint16_t addr) {
    const uint8_t msg[1] = {0};

    return i2c_write(dev, msg, 1, addr);
}
