#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>

#define I2C_NODE                DT_NODELABEL(i2c0)
static const struct device *i2c_dev = DEVICE_DT_GET(I2C_NODE);

// TODO: Complete the BME280 setup function
int setup_bme280() {
    if(!device_is_ready(i2c_dev)) {
        printk("I2C device not ready\n");
        return 1;
    }

    // finish the init setups for the BME280

    printk("BME280 setup complete\n");
    return 0;
}

/**
 * TODO: Complete the BME280 temperature reading function
 * 
 * @param temperature Pointer to store the read temperature in Celsius
 * 
 * @returns 0 on success, non-zero error code on failure
 */
int read_temperature_celsius(double *temperature) {
    // finish reading temperature from BME280 over I2C

    *temperature = 25.0; // placeholder value
    return 0;
}

/**
 * read temperature in Fahrenheit
 * 
 * @param temperature Pointer to store the read temperature in Fahrenheit
 * 
 * @returns 0 on success, non-zero error code on failure
 */
int read_temperature_fahrenheit(double *temperature) {
    double celsius;
    int ret = read_temperature_celsius(&celsius);
    if (ret != 0) {
        return ret;
    }
    *temperature = celsius * 1.8 + 32;
    return 0;
}