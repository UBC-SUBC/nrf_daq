#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/storage/disk_access.h>
#include <zephyr/fs/fs.h>

#include "disc_operations.h"

int create_new_file(const char *base_path, struct fs_file_t* file)
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

int add_data_to_file(struct fs_file_t* file, const char *data, size_t data_len)
{
	if (fs_write(file, data, data_len) < 0 ) {
		return -1;
	}

	return 0;

}

int close_file(struct fs_file_t* file)
{
	if (fs_close(file) != 0) {
		return -1; 
	}
	return 0; 
}

int lsdir(const char *path)
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