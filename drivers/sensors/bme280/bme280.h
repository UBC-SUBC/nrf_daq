#define BME_CONFIG_ADDRESS 0xF5
#define BME_I2C_ADDRESS 0x77
#define BME_PRESSUREDATA = 0xF7
#define BME_TEMPDATA = 0xFA
#define BME_HUMIDDATA = 0xFD
#define BME_DIG_T1 0x88
#define BME_DIG_T2 0x8A
#define BME_DIG_T3 0x8C
#define BME_DIG_H1 = 0xA1
#define BME_DIG_H2 = 0xE1
#define BME_DIG_H3 = 0xE3
#define BME_DIG_H4 = 0xE4
#define BME_DIG_H5 = 0xE5
#define BME_DIG_H6 = 0xE7


typedef struct bme280_data {
    time_t time;
    double temperature_c;
    double temperature_f;
    double humidity;
} bme280_data;

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

/**
 * TODO: Complete the BME280 humidity reading function
 * 
 * @param humidity Pointer to store the read humidity
 * @returns 0 on success, non-zero error code on failure
 */
int read_humidity(double *humidity);

/**
 * Format BME280 data into a JSON string
 * 
 * @param output_buffer Buffer to store the formatted string
 * @param buffer_size Size of the output buffer
 * @param data Pointer to bme280_data structure containing the data to format
 * 
 * @returns 0 on success, -1 on failure (e.g., buffer too small)
 */
int bme280_print(char* output_buffer, size_t buffer_size, bme280_data* data);
