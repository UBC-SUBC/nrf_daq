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
#include <time.h>

#include "filesystem/SD_card/disc_operations.h"
#include "sensors/bme280/bme280.h"

#define DEBUG_MODE 1

#define SLEEP_TIME_MS 1000
#define PRINT_BUFFER_SIZE 128

LOG_MODULE_REGISTER(main);

static FATFS fat_fs;

/* mounting info */
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
};

#if !DEBUG_MODE
static const char *disk_mount_pt = DISK_MOUNT_PT;
#endif

int main(void)
{
	// temperature
	bme280_data temperature_data = { 0 };
	char print_buffer[PRINT_BUFFER_SIZE];
	
	// ------ file system -------
	struct fs_file_t file;
	static const char *disk_pdrv = DISK_DRIVE_NAME;

	// code begins
	if (setup_bme280()) {
		printk("BME280 setup failed\n");
		return -1;
	}

#if DEBUG_MODE
	LOG_INF("Debug mode enabled. Logging to file");
#else
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

	// Get today's date
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	if (t == NULL) {
		LOG_ERR("Failed to get local time");
		return -1;
	}

	if (create_new_file("data.txt", &file, t) != 0) {
		LOG_ERR("Failed to create new file");
		return -1;
	}
#endif

	while (1) {
		if (read_temperature_celsius(&temperature_data.temperature_c)) {
			LOG_ERR("Failed to read temperature in Celsius\n");
			return -1;
		}

		if (read_humidity(&temperature_data.humidity)) {
			LOG_ERR("Failed to read humidity\n");
			return -1;
		}

		// Get current time
		struct timespec ts;
		if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
			LOG_ERR("Failed to get current time\n");
			return -1;
		}
		temperature_data.time = ts.tv_sec;

		// Print reading to console if in debug, otherwise write to file
		if (bme280_print(print_buffer, PRINT_BUFFER_SIZE, &temperature_data) != 0) {
			LOG_ERR("Failed to format BME280 data\n");
			return -1;
		}

	#if DEBUG_MODE
		printk("%s\n", print_buffer);
	#else
		// write something to that file
		if (add_data_to_file(&file, print_buffer, strlen(print_buffer)) != 0) {
			LOG_ERR("Failed to write data to file");
			close_file(&file);
			return -1;
		}
	#endif
		k_msleep(SLEEP_TIME_MS);
	}

#if !DEBUG_MODE
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
#endif
	return 0;
}