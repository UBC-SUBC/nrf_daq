#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <stdio.h>

#include "bme280.h"

#define I2C0_NODE DT_NODELABEL(bme280)
static const struct i2c_dt_spec i2c_dev = I2C_DT_SPEC_GET(I2C0_NODE);

int setup_bme280() {
    if(!device_is_ready(i2c_dev.bus)) {
        printk("I2C device not ready\n");
        return 1;
    }

    uint8_t config[2] = {BME_CONFIG_ADDRESS, 0x48};

    int return_val = i2c_write_dt(&i2c_dev, config, sizeof(config));
    
    if (return_val != 0){
        printk ("BME not connected");
        return -1;
    }
    else {
    printk("BME280 setup complete\n");
    }

    uint8_t sensor_id = BME_ID_ADDRESS; 
    uint8_t ret_id; 

    int ret = i2c_write_read_dt(&i2c_dev, &sensor_id, 1, &ret_id,1);

    if (ret != 0) {
        printk("Sensor ID Pull Unsuccessful"); 
        return -1; 
    }
    else if (ret_id == BME_ID) {
        printk("Successfully Pulled Sensor ID - ID: 0x%x\n", &ret_id);
    }
    else {
        printk("Incorrect Sensor ID Pulled - ID: 0x%x\n", &ret_id);
        return -1;
    }
    return 0;
}
 /** 
 * @param raw_temp - raw temperature reading
 * 
 * @returns corrected temperature 
*/

static double correct_temp (int raw_temp) {
    uint32_t calc_val1;
    uint32_t calc_val2;

    calc_val1 =(uint32_t)((raw_temp/8) - ((uint32_t)BME_DIG_T1*2));
    calc_val1 = (calc_val1 * ((uint32_t)BME_DIG_T2))/ 2048;
    calc_val2 =(uint32_t)((raw_temp/16) - ((uint32_t)BME_DIG_T1*2));
    calc_val2 = (((calc_val2 * calc_val2)/ 4096)* ((uint32_t)BME_DIG_T3)) / 16384; 

    uint32_t throwaway = calc_val1 + calc_val2;   
    uint32_t final_temp = (float)((throwaway * 5 + 128)/256);

    double corrected_temp = (double)final_temp; 

    return corrected_temp;

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
    uint8_t temp_read[3] = {0}; 
    uint8_t sensor_register = BME_TEMPDATA; 

    int readcheck = i2c_burst_read_dt(&i2c_dev, BME_TEMPDATA,temp_read,3);

    if (readcheck != 0) {
        printk("Temperature unsuccessfully read");
        return -1;
    }
    uint32_t temp_buff = 0;

    temp_buff = ((uint32_t)temp_read [0] << 12 | (uint32_t)temp_read [1] << 4 | (uint32_t)temp_read [2] >> 4);
     
    *temperature = correct_temp(temp_buff);

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


/**
 * Corrects humidity readings with calibration registers 
 * 
 * @param humid_buff Stores the humidity data
 * 
 * @returns Corrected humidity data according to calibration registers
 */
static double correct_humid (int humid_buff) {
    uint32_t var1, var2, var3, var4, var5; 

      // might not need - this is humidity calibration for sensor

    var1 = 1; // would normally be var1 = throwaway - ((uint32_t)76800);
    var2 = (int32_t)(humid_buff * 16384);
    var3 = (int32_t)(((int32_t)BME_DIG_H4) * 1048576);
    var4 = ((int32_t)BME_DIG_H5) * var1;
    var5 = (((var2 - var3) - var4) + (int32_t)16384) / 32768;
    var2 = (var1 * ((int32_t)BME_DIG_H6)) / 1024;
    var3 = (var1 * ((int32_t)BME_DIG_H3)) / 2048;
    var4 = ((var2 * (var3 + (int32_t)32768)) / 1024) + (int32_t)2097152;
    var2 = ((var4 * ((int32_t)BME_DIG_H2)) + 8192) / 16384;
    var3 = var5 * var2;
    var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
    var5 = var3 - ((var4 * ((int32_t)BME_DIG_H1)) / 16);
    var5 = (var5 < 0 ? 0 : var5);
    var5 = (var5 > 419430400 ? 419430400 : var5);
    uint32_t placeholder = (uint32_t)(var5 / 4096);

    uint32_t tempval = placeholder / 1024.0;

    double corrected_humid = (double)tempval; 

    return corrected_humid; 
} 


/** 
 * TODO: Complete the BME280 humidity reading function
 */
int read_humidity(double *humidity) {
    // finish reading humidity from BME280 over I2C 
    uint8_t humid_read[2] = {0}; 
    uint8_t sensor_register = BME_HUMIDDATA; 

    int readcheck = i2c_burst_read_dt(&i2c_dev, BME_HUMIDDATA, humid_read, 2);

    if (readcheck != 0) {
        printk("Humidity unsuccessfully read");
        return -1;
    }

    uint16_t humid_buff = 0;

    humid_buff = (humid_read[0] << 8 | humid_read[1]); 
  
    *humidity = correct_humid(humid_buff); // calls the humidity reading adjustment 

    return 0;
    }


int bme280_print(char* output_buffer, size_t buffer_size, bme280_data* data) {
    int written = snprintf(output_buffer, buffer_size,
                        "{\"t\" :%.2lld,\"sensor\":\"temp_humid\",\"temp_c\":%.2f,\"humid\":%.2f}\n",
                            data->time,
                            data->temperature_c, data->humidity);

    if (written < 0 || (size_t)written >= buffer_size) {
        return -1;
    }
    return 0;
}


// print fcn but not sure if needed 
int bme280_print_to_console(bme280_data* data) {
    char buf[128]; // buf to hold data 
    if (bme280_print(buf, sizeof(buf), data) != 0) {
        return -1; // checks if buf fits 
    }
    printk("%s", buf);
    return 0;
}


