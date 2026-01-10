#define BME_CONFIG_ADDRESS 0xF5
#define BME_I2C_ADDRESS 0x77
#define BME_DIG_T1 0x88
#define BME_DIG_T2 0x8A
#define BME_DIG_T3 0x8C


/**
 * TODO: Complete the BME280 setup function
 * 
 * @returns 0 on success, non-zero error code on failure
 */
int setup_bme280();

/**
 * TODO: Complete the BME280 temperature reading function
 * 
 * @param temperature Pointer to store the read temperature in Celsius
 * @returns 0 on success, non-zero error code on failure
 */
int read_temperature_celsius(double *temperature);

/**
 * read temperature in Fahrenheit
 * 
 * @param temperature Pointer to store the read temperature in Fahrenheit
 * @returns 0 on success, non-zero error code on failure
 */
int read_temperature_fahrenheit(double *temperature);


