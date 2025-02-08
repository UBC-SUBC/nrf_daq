#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>

#include "bno055.h"

#define I2C_NODE                DT_NODELABEL(i2c0)
static const struct device *i2c_dev = DEVICE_DT_GET(I2C_NODE);

#define READ_SENSOR_INTERVAL    50
#define ERROR_SLEEP_INTERVAL    5000

#define LED0_NODE               DT_ALIAS(led0)

static uint8_t write_i2c_buffer[3];
static uint8_t read_i2c_buffer[8];

struct bno055_t bno055;

int err;

/* READ DEVICE ID */
int read_device_id() {
    write_i2c_buffer[0] = BNO055_CHIP_ID_ADDR;
    err = i2c_write_read(i2c_dev, BNO055_I2C_ADDR1, write_i2c_buffer, 1, read_i2c_buffer, 1);
    if (err) {
        printk("Failed to read device ID\n");
        return 1;
    } else {
        bno055.device_id = read_i2c_buffer[0];
        printk("Device ID: %d\n", bno055.device_id);
    }

    return 0;
}

/* SET CONFIG MODE */
int set_config_mode() {
    write_i2c_buffer[0] = BNO055_OPR_MODE_ADDR;
    write_i2c_buffer[1] = BNO055_OPERATION_MODE_CONFIG;
    err = i2c_write(i2c_dev, write_i2c_buffer, 2, BNO055_I2C_ADDR1);
    if (err) {
        printk("Failed to set config mode\n");
        return 1;
    } else {
        printk("Config mode set\n");
        return 0;
    }
}

/* SET PAGE ID 0 */
int set_page_id() {
    write_i2c_buffer[0] = BNO055_PAGE_ID_ADDR;
    write_i2c_buffer[1] = BNO055_PAGE_ID_POS;
    err = i2c_write(i2c_dev, write_i2c_buffer, 2, BNO055_I2C_ADDR1);
    if (err) {
        printk("Failed to set page ID\n");
        return 1;
    } else {
        printk("Page ID set\n");
        return 0;
    }
}

/* SET EXTERNAL CRYSTAL */
int set_external_crystal() {
    write_i2c_buffer[0] = BNO055_SYS_TRIGGER_ADDR;
    write_i2c_buffer[1] = 0x80;
    err = i2c_write(i2c_dev, write_i2c_buffer, 2, BNO055_I2C_ADDR1);
    if (err) {
        printk("Failed to set external crystal\n");
        return 1;
    } else {
        printk("External crystal set\n");
        return 0;
    }
}

/* SET OPMODE */
int set_opmode() {
    write_i2c_buffer[0] = BNO055_OPERATION_MODE_REG;
    write_i2c_buffer[1] = BNO055_OPERATION_MODE_NDOF;
    err = i2c_write(i2c_dev, write_i2c_buffer, 2, BNO055_I2C_ADDR1);
    if (err) {
        printk("Failed to set operation mode\n");
        return 1;
    } else {
        printk("Operation mode set\n");
        return 0;
    }
}

int setup_bno055() {
    if(!device_is_ready(i2c_dev)) {
        printk("I2C device not ready\n");
        return 1;
    }

    if(read_device_id()) {
        return 1;
    }

    bno055.is_calibrated = false;

    if(set_config_mode()) {
        return 1;
    }
    if(set_page_id()) {
        return 1;
    }
    if(set_external_crystal()) {
        return 1;
    }
    if(set_opmode()) {
        return 1;
    }

    set_calibration();
    printk("BNO055 setup complete\n");
    return 0;
}

/* READ QUATERNION DATA */
void read_quaternion_xyz(struct bno055_quaternion_t *quat) {
    write_i2c_buffer[0] = BNO055_QUATERNION_DATA_W_LSB_ADDR;
    err = i2c_write_read(i2c_dev, BNO055_I2C_ADDR1, write_i2c_buffer, 1, read_i2c_buffer, 8);
    if (err) {
        printk("Failed to read quaternion data\n");
        return;
    } else {
        quat->w = (read_i2c_buffer[1] << 8) | read_i2c_buffer[0];
        quat->x = (read_i2c_buffer[3] << 8) | read_i2c_buffer[2];
        quat->y = (read_i2c_buffer[5] << 8) | read_i2c_buffer[4];
        quat->z = (read_i2c_buffer[7] << 8) | read_i2c_buffer[6];
        printk("Quaternion Data - W: %6d, X: %6d, Y: %6d, Z: %6d\n", quat->w, quat->x, quat->y, quat->z);
    }
}

void read_gyro_xyz(struct bno055_gyro_t *gyro) {
    write_i2c_buffer[0] = BNO055_GYRO_DATA_X_LSB_ADDR;
    err = i2c_write_read(i2c_dev, BNO055_I2C_ADDR1, write_i2c_buffer, 1, read_i2c_buffer, BNO055_GYRO_XYZ_DATA_SIZE);
    if (err) {
        printk("Failed to read gyro data\n");
        return;
    } else {
        gyro->x = (read_i2c_buffer[1] << 8) | read_i2c_buffer[0];
        gyro->y = (read_i2c_buffer[3] << 8) | read_i2c_buffer[2];
        gyro->z = (read_i2c_buffer[5] << 8) | read_i2c_buffer[4];
        printk("Gyro Data - X: %6d, Y: %6d, Z: %6d\n", gyro->x, gyro->y, gyro->z);
    }
}

void read_euler_hrp(struct bno055_euler_t *euler) {
    write_i2c_buffer[0] = BNO055_EULER_H_LSB_ADDR;
    err = i2c_write_read(i2c_dev, BNO055_I2C_ADDR1, write_i2c_buffer, 1, read_i2c_buffer, BNO055_EULER_HRP_DATA_SIZE);
    if (err) {
        printk("Failed to read euler data\n");
        return;
    } else {
        uint16_t h = (read_i2c_buffer[1] << 8) | read_i2c_buffer[0];
        uint16_t r = (read_i2c_buffer[3] << 8) | read_i2c_buffer[2];
        uint16_t p = (read_i2c_buffer[5] << 8) | read_i2c_buffer[4];
        euler->h = bno055.is_calibrated ? h - bno055.euler_cal.h : h;
        euler->r = bno055.is_calibrated ? r - bno055.euler_cal.r : r;
        euler->p = bno055.is_calibrated ? p - bno055.euler_cal.p : p;
        printk("Euler Data - H: %6d, R: %6d, P: %6d\n", euler->h, euler->r, euler->p);
    }
}

void set_calibration() {
    read_euler_hrp(&bno055.euler_cal);
    while(bno055.euler_cal.h == 0 && bno055.euler_cal.r == 0 && bno055.euler_cal.p == 0) {
        printk("Calibration not complete\n");
        k_sleep(K_MSEC(ERROR_SLEEP_INTERVAL));
        read_euler_hrp(&bno055.euler_cal);
    }
    bno055.is_calibrated = true;
}