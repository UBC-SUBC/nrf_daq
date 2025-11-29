/*
 * Copyright (c) 2019 Tavish Naruka <tavishnaruka@gmail.com>
 * Copyright (c) 2023 Nordic Semiconductor ASA
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Sample which uses the filesystem API and SDHC driver */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>
#include <ff.h>

#include "filesystem/SD_card/disc_operations.h"
#include "sensors/bme280/bme280.h"

LOG_MODULE_REGISTER(main);

#define PATH_LENGTH 256

/* Data structure to store BME280 data */
struct bme280_data {
	uint16_t dig_t1;
	int16_t dig_t2;
	int16_t dig_t3;
} bmedata;

void bme_calibrationdata(const struct i2c_dt_spec *spec, struct bme280_data *sensor_data_ptr)
{
	
	/* Step 10 - Put calibration function code */
	uint8_t values[6];

	int ret = i2c_burst_read_dt(spec, CALIB00, values, 6);

	if (ret != 0) {
		printk("Failed to read register %x \n", CALIB00);
		return;
	}

	sensor_data_ptr->dig_t1 = ((uint16_t)values[1]) << 8 | values[0];
	sensor_data_ptr->dig_t2 = ((uint16_t)values[3]) << 8 | values[2];
	sensor_data_ptr->dig_t3 = ((uint16_t)values[5]) << 8 | values[4];
}

/* Compensate current temperature using previously stored sensor calibration data */
static int32_t bme280_compensate_temp(struct bme280_data *data, int32_t adc_temp)
{
	int32_t var1, var2;

	var1 = (((adc_temp >> 3) - ((int32_t)data->dig_t1 << 1)) * ((int32_t)data->dig_t2)) >> 11;

	var2 = (((((adc_temp >> 4) - ((int32_t)data->dig_t1)) *
		  ((adc_temp >> 4) - ((int32_t)data->dig_t1))) >>
		 12) *
		((int32_t)data->dig_t3)) >>
	       14;

	return ((var1 + var2) * 5 + 128) >> 8;
}

static FATFS fat_fs;
/* mounting info */
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
};
static const char *disk_mount_pt = DISK_MOUNT_PT;

int main(void)
{
	struct fs_file_t file;
	static const char *disk_pdrv = DISK_DRIVE_NAME;
	static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_NODE);

	if (!device_is_ready(dev_i2c.bus)) {
		printk("I2C bus %s is not ready!\n", dev_i2c.bus->name);
		return -1;
	}

	/* STEP 9 - Verify it is proper device by reading device id  */
	uint8_t id = 0;
	uint8_t regs[] = {ID};

	int ret = i2c_write_read_dt(&dev_i2c, regs, 1, &id, 1);

	if (ret != 0) {
		printk("Failed to read register %x \n", regs[0]);
		return -1;
	}

	if (id != CHIP_ID) {
		printk("Invalid chip id! %x \n", id);
		return -1;
	}

	bme_calibrationdata(&dev_i2c, &bmedata);

	/* STEP 11 - Setup the sensor by writing the value 0x93 to the Configuration register */
	uint8_t sensor_config[] = {CTRLMEAS, SENSOR_CONFIG_VALUE};

	ret = i2c_write_dt(&dev_i2c, sensor_config, 2);

	if (ret != 0) {
		printk("Failed to write register %x \n", sensor_config[0]);
		return -1;
	}

	while (1) {

		/* STEP 12 - Read the temperature from the sensor */
		uint8_t temp_val[3] = {0};

		int ret = i2c_burst_read_dt(&dev_i2c, TEMPMSB, temp_val, 3);

		if (ret != 0) {
			printk("Failed to read register %x \n", TEMPMSB);
			k_msleep(SLEEP_TIME_MS);
			continue;
		}

		/* STEP 12.1 - Put the data read from registers into actual order (see datasheet) */
		int32_t adc_temp =
			(temp_val[0] << 12) | (temp_val[1] << 4) | ((temp_val[2] >> 4) & 0x0F);

		/* STEP 12.2 - Compensate temperature */
		int32_t comp_temp = bme280_compensate_temp(&bmedata, adc_temp);

		/* STEP 12.3 - Convert temperature */
		float temperature = (float)comp_temp / 100.0f;
		double fTemp = (double)temperature * 1.8 + 32;

		// Print reading to console
		printk("Temperature in Celsius : %8.2f C\n", (double)temperature);
		printk("Temperature in Fahrenheit : %.2f F\n", fTemp);

		k_msleep(SLEEP_TIME_MS);
	}

	// END OF TEMP SENSOR

	// open disk
	if (disk_access_init(disk_pdrv)) {
		LOG_ERR("Disk init failed");
		return -1;
	}

	// mount disk
	mp.mnt_point = disk_mount_pt;
	if (fs_mount(&mp)) {
		LOG_ERR("Error mounting disk");
		return -1;
	}

	printk("Mounting disk at %s\n", disk_mount_pt);

	// list all files and directories on the SD card
	lsdir(disk_mount_pt);

	// add one file
	if (create_new_file("rotation_data.txt", &file) != 0) {
		LOG_ERR("Failed to create new file");
		return -1;
	}

	// write something to that file
	char* data = "Sample data to write";
	if (add_data_to_file(&file, data, strlen(data)) != 0) {
		LOG_ERR("Failed to write data to file");
		close_file(&file);
		return -1;
	}

	// close that file
	if (close_file(&file) != 0) {
		LOG_ERR("Failed to close file");
		return -1;
	}

	// flush data to disk
	if (disk_access_ioctl(disk_pdrv,
			DISK_IOCTL_CTRL_SYNC, NULL) != 0) {
		LOG_ERR("Storage deinit ERROR!");
		return -1;
	}

	// unmount/close disk
	fs_unmount(&mp);

	return 0;
}