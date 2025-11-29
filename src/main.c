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

#define DISK_DRIVE_NAME "SD"

#define DISK_MOUNT_PT "/"DISK_DRIVE_NAME":"

static FATFS fat_fs;
/* mounting info */
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
};

#define FS_RET_OK FR_OK
LOG_MODULE_REGISTER(main);

#define PATH_LENGTH 256;
#define SOME_FILE_NAME "some.dat"
#define SOME_DIR_NAME "some"
#define SOME_REQUIRED_LEN MAX(sizeof(SOME_FILE_NAME), sizeof(SOME_DIR_NAME))

static int lsdir(const char *path);

//implement this function to create one file
// return 0 if success, -1 if fail
static int create_new_file(const char *base_path, struct fs_file_t* file)
{
	// definitions
	char path_buffer[PATH_LENGTH];
	struct fs_file_t file; // not sure 
	int base_length = strlen(base_path);
	// no check for overflow because assuming we have enough space

	strcpy(path_buffer,base_path);

	path_buffer[base_length] = '/';
	base_length++;
	path_buffer[base_length] = 0;

	strcat(&path_buffer[base_length], SOME_FILE_NAME);

	fs_file_t_init(&file);

	if(fs_open(&file, base_path, FS_O_CREATE) != 0)
	{
		return -1; 
	} 
	else {
		return 0; 
	}
}
// write a function to add some data into the file 
static int add_data_to_file(struct fs_file_t* file, const char *data, size_t data_len)
{
	if (fs_write(file, data, data_len) < 0 ) {
		return -1;
	}

	return 0;

}

// write a function to close a file 
static int close_file(struct fs_file_t* file)
{
	if (fs_close(file) != 0) {
		return -1; 
	}
	return 0; 
}

static const char *disk_mount_pt = DISK_MOUNT_PT;

int main(void)
{
	struct fs_file_t file;
	static const char *disk_pdrv = DISK_DRIVE_NAME;
	
	// open disk


	// mount disk (may be optional)

	// add one file
	if (create_new_file("rotation_data.txt", &file) != 0) {
		LOG_ERR("Failed to create new file");
		return -1;
	}

	DISK_MOUNT_PT + "/rotation_data.txt"
	"/SD:/rotation_data.txt"

	// write something to that file
	if (add_data_to_file(&file, "Sample data to write", 20) != 0) {
		LOG_ERR("Failed to write data to file");
		close_file(&file);
		return -1;
	}

	// close that file
	if (close_file(&file) != 0) {
		LOG_ERR("Failed to close file");
		return -1;
	}

	// unmount/close disk



	/* sample code */
	do {
		static const char *disk_pdrv = DISK_DRIVE_NAME;
		uint64_t memory_size_mb;
		uint32_t block_count;
		uint32_t block_size;

		// if (disk_access_ioctl(disk_pdrv,
		// 		DISK_IOCTL_CTRL_INIT, NULL) != 0) {
		// 	LOG_ERR("Storage init ERROR!");
		// 	break;
		// }

		if (disk_access_ioctl(disk_pdrv,
				DISK_IOCTL_GET_SECTOR_COUNT, &block_count)) {
			LOG_ERR("Unable to get sector count");
			break;
		}
		LOG_INF("Block count %u", block_count);

		if (disk_access_ioctl(disk_pdrv,
				DISK_IOCTL_GET_SECTOR_SIZE, &block_size)) {
			LOG_ERR("Unable to get sector size");
			break;
		}
		printk("Sector size %u\n", block_size);

		memory_size_mb = (uint64_t)block_count * block_size;
		printk("Memory Size(MB) %u\n", (uint32_t)(memory_size_mb >> 20));

		// if (disk_access_ioctl(disk_pdrv,
		// 		DISK_IOCTL_CTRL_DEINIT, NULL) != 0) {
		// 	LOG_ERR("Storage deinit ERROR!");
		// 	break;
		// }
	} while (0);

	mp.mnt_point = disk_mount_pt;

	int res = fs_mount(&mp);

	if (res == FS_RET_OK) {
		printk("Disk mounted.\n");
		/* Try to unmount and remount the disk */
		res = fs_unmount(&mp);
		if (res != FS_RET_OK) {
			printk("Error unmounting disk\n");
			return res;
		}
		res = fs_mount(&mp);
		if (res != FS_RET_OK) {
			printk("Error remounting disk\n");
			return res;
		}

		if (lsdir(disk_mount_pt) == 0) {
			if (create_some_entries(disk_mount_pt)) {
				lsdir(disk_mount_pt);
			}
		}
	} else {
		printk("Error mounting disk.\n");
	}

	fs_unmount(&mp);

	while (1) {
		k_sleep(K_MSEC(1000));
	}
	return 0;
}

/* SAMPLE CODE: List dir entry by path
 *
 * @param path Absolute path to list
 *
 * @return Negative errno code on error, number of listed entries on
 *         success.
 */
static int lsdir(const char *path)
{
	int res;
	struct fs_dir_t dirp;
	static struct fs_dirent entry;
	int count = 0;

	fs_dir_t_init(&dirp);

	/* Verify fs_opendir() */
	res = fs_opendir(&dirp, path);
	if (res) {
		printk("Error opening dir %s [%d]\n", path, res);
		return res;
	}

	printk("\nListing dir %s ...\n", path);
	for (;;) {
		/* Verify fs_readdir() */
		res = fs_readdir(&dirp, &entry);

		/* entry.name[0] == 0 means end-of-dir */
		if (res || entry.name[0] == 0) {
			break;
		}

		if (entry.type == FS_DIR_ENTRY_DIR) {
			printk("[DIR ] %s\n", entry.name);
		} else {
			printk("[FILE] %s (size = %zu)\n",
				entry.name, entry.size);
		}
		count++;
	}

	/* Verify fs_closedir() */
	fs_closedir(&dirp);
	if (res == 0) {
		res = count;
	}

	return res;
}