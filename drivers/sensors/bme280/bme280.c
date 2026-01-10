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

    uint8_t config[2] = {BME_CONFIG_ADDRESS, 0x48};

    int return_val = i2c_write_dt(&i2c_dev, config, sizeof(config));
    
    if (return_val != 0){
        printk ("BME not connected")
        return -1
    }
    else {
    printk("BME280 setup complete\n");
        return 0;
    }
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
    uint32_t calc_val1;
    uint32_t calc_val2; 
    uint32_t temp_read = {0}; 
    uint8_t sensor_register = BME_I2C_ADDRESS; 

    int readcheck = i2c_write_read_dt(&i2c_dev, &sensor_register, 3, &temp_read, 3);

    if (readcheck != 0) {
        printk("Temperature unsuccessfully read");
        return -1;
    }

    temp_read = temp_read >> 4; 

    calc_val1 =(uint32_t)((temp_read/8) - ((uint32_t)BME_DIG_T1*2));
    calc_val1 = (calc_val1 * ((uint32_t)BME_DIG_T2))/ 2048;
    calc_val2 =(uint32_t)((temp_read/16) - ((uint32_t)BME_DIG_T1*2));
    calc_val2 = (((calc_val2 * calc_val2)/ 4096)* ((uint32_t)BME_DIG_T3)) / 16384; 

    uint32_t throwaway = calc_val1 + calc_val2;
    uint32_t final_temp = (float)((throwaway * 5 +128)/256);

    *temperature = final_temp; 
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
        return -1;
    }
    *temperature = celsius * 1.8 + 32;
    return 0;
}