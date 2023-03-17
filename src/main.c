#include <zephyr/kernel.h>
#include "zephyr/drivers/i2c.h"
#include "i2c_utils.h"

#define I2C_P1 DT_NODELABEL(i2c1)

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
}
