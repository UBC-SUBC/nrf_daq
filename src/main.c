#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>

#include "bar30/bar30.h"

#define SLEEP_TIME_MS 1000

#define GRAV 9.80665
#define WATER_DENSITY 997.0474 // Fresh Water
#define ATM_PRESSURE 101500

#define I2C_NODE DT_NODELABEL(mysensor)

int main(void)
{
    static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_NODE);

    // Check if i2c communication is ready
    if (!device_is_ready(dev_i2c.bus)) {
        printk("I2C bus not ready\n");
        return -1;
    }
    // Check if bar30 is initialized and ready
    if (bar30_init(&dev_i2c) != 0) {
        printk("Initialization failed\n");
        return -1;
    }

    // Read pressure and convert to depth (m)
    while (1) {
        int32_t pressure_pa;

        if (bar30_read_pressure_pa(&dev_i2c, &pressure_pa) == 0) {
            double depth = (pressure_pa - ATM_PRESSURE) / (GRAV * WATER_DENSITY);

            printk("Pressure: %d Pa\n", pressure_pa);
            printk("Depth: %.2lf m\n", depth);
        } else {
            printk("Read failed\n");
        }

        k_msleep(SLEEP_TIME_MS);
    }

    return 0;
}