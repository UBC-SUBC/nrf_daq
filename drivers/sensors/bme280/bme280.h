#define BME_CONFIG_ADDRESS 0xF5
#define BME_I2C_ADDRESS 0x77 // 
#define BME_PRESSUREDATA  0xF7 // MSB of pressure
#define BME_TEMPDATA 0xFA // MSB of temp
#define BME_HUMIDDATA 0xFD // MSB of humid
#define BME_TEMP_CALIB_REG 0x88
#define BME_HUMID_CALIB_REG 0xA1 // address of dig_h1 (start of humid calib addresses)
#define BME_HUMID_2 0xE1 // 2nd address
#define BME_ID_ADDRESS 0xD0 
#define BME_ID 0x60 // correct bme id

extern double* t_fine; 

typedef struct bme280_data {
    time_t time;
    double temperature_c;
    double temperature_f;
    double humidity;
} bme280_data;

// Contains temperature calibration data stored in BME280 registers
typedef struct temp_calib_data {
    uint16_t dig_t1;
    int16_t dig_t2;
    int16_t dig_t3;
} calibdata;

// Contains humid. calibration data stored in BME280 registers
typedef struct humid_calib_data {
    uint8_t dig_h1; 
    uint16_t dig_h2; 
    uint8_t dig_h3;
    uint16_t dig_h4;
    uint16_t dig_h5;
    uint8_t dig_h6;
} humiddata;


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
 * Corrects humidity readings with calibration registers 
 * 
 * @param humid_buff Stores the humidity data
 * 
 * @returns Corrected humidity data according to calibration registers
*/
 double humid_correction (struct humid_calib_data *data_ptr,uint16_t humid_buff);



/**
 * 
 * @param temp_buff Corrects read temperature with calibration data pulled from sensor 
 * 
 * @returns the corrected data to the read temp func
 */
 double temp_correction (struct temp_calib_data *data_ptr, uint32_t temp_buff);



 /** 
 * @param humid_calib_data Ptr to a struct that will contain the humid calib data
 * 
 * @returns struct containing calib data from regs
*/

void humid_calib_data_reg (struct humid_calib_data *ptr);



/** 
 * @param temp_calib_data Ptr to a struct that will contain the temp calib data
 * 
 * @returns struct containing calib data from regs
*/
void temp_calib_data_reg (struct temp_calib_data *ptr);



/** 
 * @param bme280_data Ptr to a struct that contains temperature info
 * 
 * @returns 0 on pass, other on fail
*/

int bme280_print_to_console(bme280_data* data);



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
