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
 * @param temp_calib_data Ptr to a struct that will contain the temp calib data
 * 
 * @returns struct containing calib data from regs
*/

void temp_calib_data_reg (struct temp_calib_data *ptr) {
    uint8_t calib_val[6]; 

    int ret = i2c_burst_read_dt(&i2c_dev, BME_TEMP_CALIB_REG, calib_val, 6);

    if (ret != 0) {
		printk("Failed to retrieve calibration data");
		return;
	}

	ptr->dig_t1 = ((uint16_t)calib_val[1]) << 8 | calib_val[0];
	ptr->dig_t2 = ((uint16_t)calib_val[3]) << 8 | calib_val[2];
	ptr->dig_t3 = ((uint16_t)calib_val[5]) << 8 | calib_val[4];

}

 /** 
 * @param humid_calib_data Ptr to a struct that will contain the humid calib data
 * 
 * @returns struct containing calib data from regs
*/
void humid_calib_data_reg (struct humid_calib_data *ptr) {
    uint8_t val_h1; 

    int ret1 = i2c_burst_read_dt(&i2c_dev,BME_HUMID_CALIB_REG, val_h1,1);

    ptr->dig_h1 = val_h1;  

    uint8_t vals_hx[7]; 

    int ret2 = i2c_burst_read_dt(&i2c_dev, BME_HUMID_2, vals_hx, 7);

    ptr->dig_h2 = ((uint16_t)vals_hx[0] || (uint16_t)vals_hx[1] << 8 ); 
    ptr->dig_h3 = vals_hx[2]; 
    ptr->dig_h4 = ((uint16_t)vals_hx[3] || (uint16_t)vals_hx[4] << 4); 
    ptr->dig_h5 = ((uint16_t)vals_hx[5] || (uint16_t)vals_hx[6] << 4);
    ptr->dig_h6 = vals_hx[7]; 

}

/**
 * 
 * @param temp_buff Corrects read temperature with calibration data pulled from sensor 
 * @returns the corrected data to the read temp func
 */
 double temp_correction (struct temp_calib_data *data_ptr, uint32_t temp_buff) {
    int32_t var1;
    int32_t var2; 

    
	var1 = (((temp_buff >> 3) - ((int32_t)data_ptr->dig_t1 << 1)) * ((int32_t)data_ptr->dig_t2)) >> 11;

	var2 = (((((temp_buff >> 4) - ((int32_t)data_ptr->dig_t1)) *
		  ((temp_buff >> 4) - ((int32_t)data_ptr->dig_t1))) >>12) *
		((int32_t)data_ptr->dig_t3)) >> 14;

    *t_fine = (double)(var1 + var2);
	return ((var1 + var2) * 5 + 128) >> 8;
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

    int readcheck = i2c_burst_read_dt(&i2c_dev, BME_TEMPDATA,temp_read,3);

    if (readcheck != 0) {
        printk("Temperature unsuccessfully read");
        return -1;
    }
    uint32_t temp_buff = 0;

    temp_buff = ((uint32_t)temp_read [0] << 12 | (uint32_t)temp_read [1] << 4 | (uint32_t)temp_read [2] >> 4);
    
    struct temp_calib_data calibdata;
    temp_calib_data_reg(&calibdata);
     
    *temperature = temp_correction(&calibdata,temp_buff);

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
 double humid_correction (struct humid_calib_data *data_ptr,uint16_t humid_buff) {

    double h = *t_fine - (double)76800.0;
    h = (humid_buff - (((double)data_ptr->dig_h4 * 64.0)+ ((double)data_ptr->dig_h5/16384.0)* h))
    * (((double)data_ptr->dig_h2 * 65536.0)*(1.0 + (((double)data_ptr->dig_h3) / 67108864.0) * h *
        (1.0 + ((double)data_ptr->dig_h6) / 67108864.0f * h)));

    h = h * (1.0 - ((double)data_ptr->dig_h1 * h / 524288.0));

    return h; 

} 

/** 
 * TODO: Complete the BME280 humidity reading function
 */
int read_humidity(double *humidity) {
    // finish reading humidity from BME280 over I2C 

    uint8_t humid_read[2] = {0}; 

    int readcheck = i2c_burst_read_dt(&i2c_dev, BME_HUMIDDATA, humid_read, 2);

    if (readcheck != 0) {
        printk("Humidity unsuccessfully read");
        return -1;
    }

    uint16_t humid_buff = 0;

    humid_buff = (humid_read[0] << 8 | humid_read[1]);
    
    struct humid_calib_data humiddata;
    humid_calib_data_reg(&humiddata);
  
    *humidity = humid_correction(&humiddata, humid_buff); // calls the humidity reading adjustment 

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


