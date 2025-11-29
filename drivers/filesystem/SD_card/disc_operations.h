

#define DISK_DRIVE_NAME "SD"
#define DISK_MOUNT_PT "/"DISK_DRIVE_NAME":"
#define PATH_LENGTH 256

int create_new_file(const char *base_path, struct fs_file_t* file);
int lsdir(const char *path);
int add_data_to_file(struct fs_file_t* file, const char *data, size_t data_len);
int close_file(struct fs_file_t* file);

