/*
 * Copyright (c) 2019 Tavish Naruka <tavishnaruka@gmail.com>
 * Copyright (c) 2023 Nordic Semiconductor ASA
 * Copyright (c) 2023 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Sample which uses the filesystem API and SDHC driver */

#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>
#include <ff.h>
#include <time.h>

#include "filesystem/SD_card/disc_operations.h"
#include "../drivers/sensors/custom_bme280/custom_bme280.h"

#define DEBUG_MODE 1

#define SLEEP_TIME_MS 1000
#define PRINT_BUFFER_SIZE 128

LOG_MODULE_REGISTER(main);

#if !DEBUG_MODE
static FATFS fat_fs;

/* mounting info */
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
};

static const char *disk_mount_pt = DISK_MOUNT_PT;
#endif

const struct device * dev = DEVICE_DT_GET(DT_NODELABEL(bme280));

int main(void)
{
	int err = 0;
	struct sensor_value temp_val, press_val, hum_val;
	char print_buffer[PRINT_BUFFER_SIZE];

#if DEBUG_MODE
	LOG_INF("Debug mode enabled. Logging to file");
#else
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

	err = device_is_ready(dev);
	if (!err) {
		LOG_INF("Error: SPI device is not ready, err: %d", err);
		return 0;
	}

	while (1) {
		/* STEP 17.2 - Continuously read out sensor data using the sensor API calls */
		err = sensor_sample_fetch(dev);
		if (err < 0) {
			LOG_ERR("Could not fetch sample (%d)", err);
			return 0;
		}

		if (sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp_val)) {
			LOG_ERR("Could not get sample");
			return 0;
		}
		
		if (sensor_channel_get(dev, SENSOR_CHAN_PRESS, &press_val)) {
			LOG_ERR("Could not get sample");
			return 0;
		}
	
		if (sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &hum_val)) {
			LOG_ERR("Could not get sample");
			return 0;
		}

		LOG_INF("Compensated temperature value: %d", temp_val.val1);
		LOG_INF("Compensated pressure value: %d", press_val.val1);
		LOG_INF("Compensated humidity value: %d", hum_val.val1);
		
		k_sleep(K_MSEC(1000));

		struct timespec ts;
		if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
			LOG_ERR("Failed to get current time\n");
			return -1;
		}

		// Print reading to console if in debug, otherwise write to file
		if (bme280_print(print_buffer, PRINT_BUFFER_SIZE, ts.tv_sec, &temp_val, &hum_val) != 0) {
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