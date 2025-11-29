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

#define FS_RET_OK FR_OK
LOG_MODULE_REGISTER(main);

#define PATH_LENGTH 256

static FATFS fat_fs;
/* mounting info */
static struct fs_mount_t mp = {
	.type = FS_FATFS,
	.fs_data = &fat_fs,
};

static int lsdir(const char *path);

//implement this function to create one file
// return 0 if success, -1 if fail
static int create_new_file(const char *base_path, struct fs_file_t* file)
{
	// definitions
	char path_buffer[PATH_LENGTH];
	int base_length = strlen(base_path);
	int sdLength = strlen (DISK_MOUNT_PT);
	// no check for overflow because assuming we have enough space
	int overflowCheck = base_length + sdLength; 

	if (overflowCheck > PATH_LENGTH){
		return 1; 			// 1 indicates that the path is too long
	}
	
	strcpy(path_buffer, DISK_MOUNT_PT);

	path_buffer[sdLength] = '/';
	sdLength++;
	path_buffer[sdLength] = 0;

	strcat(&path_buffer[sdLength],base_path);

	fs_file_t_init(file);

	if(fs_open(file, path_buffer, FS_O_CREATE) != 0)
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

/* List dir entry by path
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