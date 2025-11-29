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
#include <zephyr/devicetree.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>
#include <zephyr/drivers/i2c.h>
#include <ff.h>

#include "filesystem/SD_card/disc_operations.h"
#include "sensors/bme280/bme280.h"

 #define SLEEP_TIME_MS 1000
 
 /* STEP 8 - Define the I2C slave device address and the addresses of relevant registers */
 #define STTS751_TEMP_HIGH_REG 0x00
 #define STTS751_TEMP_LOW_REG  0x02
 #define STTS751_CONFIG_REG    0x03
 
 /* STEP 6 - Get the node identifier of the sensor */
 #define I2C_NODE DT_NODELABEL(mysensor)
 

LOG_MODULE_REGISTER(main);

static FATFS fat_fs;
/* mounting info */
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
};
static const char *disk_mount_pt = DISK_MOUNT_PT;

int main(void)
{
	double temperature_c = 0.0;
	double temperature_f = 0.0;

	if (setup_bme280()) {
		printk("BME280 setup failed\n");
		return -1;
	}

	/* demo code for setting up I2C device
	static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_NODE);
	if (!device_is_ready(dev_i2c.bus)) {
		printk("I2C bus %s is not ready!\n", dev_i2c.bus->name);
		return -1;
	}

	uint8_t config[2] = {STTS751_CONFIG_REG, 0x8C};
	ret = i2c_write_dt(&dev_i2c, config, sizeof(config));
	if (ret != 0) {
		printk("Failed to write to I2C device address %x at Reg. %x \n", dev_i2c.addr,
			config[0]);
		return -1;
	}
	*/

	while (1) {
		if (read_temperature_celsius(&temperature_c)) {
			printk("Failed to read temperature in Celsius\n");
			return -1;
		}
		if (read_temperature_fahrenheit(&temperature_f)) {
			printk("Failed to read temperature in Fahrenheit\n");
			return -1;
		}
		
		/* demo code for reading temperature
		uint8_t temp_reading[2] = {0};
		uint8_t sensor_regs[2] = {STTS751_TEMP_LOW_REG, STTS751_TEMP_HIGH_REG};
		ret = i2c_write_read_dt(&dev_i2c, &sensor_regs[0], 1, &temp_reading[0], 1);
		if (ret != 0) {
			printk("Failed to write/read I2C device address %x at Reg. %x \n",
				dev_i2c.addr, sensor_regs[0]);
		}
		ret = i2c_write_read_dt(&dev_i2c, &sensor_regs[1], 1, &temp_reading[1], 1);
		if (ret != 0) {
			printk("Failed to write/read I2C device address %x at Reg. %x \n",
				dev_i2c.addr, sensor_regs[1]);
		}

		int temp = ((int)temp_reading[1] * 256 + ((int)temp_reading[0] & 0xF0)) / 16;
		if (temp > 2047) {
			temp -= 4096;
		}

		// Convert to engineering units
		double cTemp = temp * 0.0625;
		double fTemp = cTemp * 1.8 + 32;
		*/

		// Print reading to console
		printk("Temperature in Celsius : %.2f C \n", temperature_c);
		printk("Temperature in Fahrenheit : %.2f F \n", temperature_f);
		k_msleep(SLEEP_TIME_MS);
	}
	// ------ file system -------
	struct fs_file_t file;
	static const char *disk_pdrv = DISK_DRIVE_NAME;
	
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