/**
 * Author: Jocelyn Zhao
 * Feb 28, 2026
 */

#ifndef CUSTOM_BME280_H
#define CUSTOM_BME280_H

#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <time.h>

#define DELAY_REG 		10
#define DELAY_PARAM		50
#define DELAY_VALUES	1000
#define LED0	13

#define CTRLHUM 		0xF2
#define CTRLMEAS		0xF4
#define CALIB00			0x88
#define CALIB24			0xA1
#define CALIB26			0xE1
#define ID				0xD0
#define PRESSMSB		0xF7
#define PRESSLSB		0xF8
#define PRESSXLSB		0xF9
#define TEMPMSB			0xFA
#define TEMPLSB			0xFB
#define TEMPXLSB		0xFC
#define HUMMSB			0xFD
#define HUMLSB			0xFE
#define DUMMY			0xFF

#define BME280_CHIP_ID  0x60
#define REG_STATUS 		0xF3

#define STATUS_MEASURING 	0x08
#define STATUS_IM_UPDATE 	0x01

#define NUM_BYTES_ALL_DATA. 8

struct custom_bme280_data {
	/* Compensation parameters */
	uint16_t dig_t1;
	int16_t dig_t2;
	int16_t dig_t3;
	uint16_t dig_p1;
	int16_t dig_p2;
	int16_t dig_p3;
	int16_t dig_p4;
	int16_t dig_p5;
	int16_t dig_p6;
	int16_t dig_p7;
	int16_t dig_p8;
	int16_t dig_p9;
	uint8_t dig_h1;
	int16_t dig_h2;
	uint8_t dig_h3;
	int16_t dig_h4;
	int16_t dig_h5;
	int8_t dig_h6;

	/* Compensated values */
	int32_t comp_temp;
	uint32_t comp_press;
	uint32_t comp_humidity;

	/* Carryover between temperature and pressure/humidity compensation */
	int32_t t_fine;

	uint8_t chip_id;
};

struct custom_bme280_config {
	struct i2c_dt_spec i2c;
};

int bme280_print(char *buf, size_t buf_size, time_t time,
		 struct sensor_value *temperature,
		 struct sensor_value *humidity);

#endif /* CUSTOM_BME280_H */
