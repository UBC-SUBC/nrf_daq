#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include "sensor/bno055/bno055.h"
#include "sensor/bno055/driver_functions.h"
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>

#define BNO055_I2C_ADDR 0x28  // Adjust based on your hardware
#define BNO055_OPR_MODE_ADDR 0x3D
#define BNO055_SYS_TRIGGER_ADDR 0x3F
#define BNO055_EULER_H_LSB_ADDR 0x1A
#define BNO055_OPR_MODE_NDOF 0x0C
#define BNO055_SYS_RESET 0x20

#if !DT_HAS_COMPAT_STATUS_OKAY(bosch_bno055)
#error "No bosch,bno055 compatible node found in the device tree"
#endif

const struct device* i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c0));

void bno055_reset(void)
{
    uint8_t reset_data[2] = {BNO055_SYS_TRIGGER_ADDR, BNO055_SYS_RESET};
    int ret = i2c_write(i2c_dev, reset_data, sizeof(reset_data), BNO055_I2C_ADDR);
    if (ret < 0) {
        printk("Failed to reset BNO055\n");
    } else {
        printk("BNO055 reset successfully\n");
    }
    k_sleep(K_MSEC(650));  // Wait for reset
}

int bno055_set_mode(uint8_t mode)
{
    uint8_t mode_data[2] = {BNO055_OPR_MODE_ADDR, mode};
    int ret = i2c_write(i2c_dev, mode_data, sizeof(mode_data), BNO055_I2C_ADDR);
    if (ret < 0) {
        printk("Failed to set BNO055 mode\n");
        return ret;
    }
    k_sleep(K_MSEC(30));  // Wait for mode change
    return 0;
}

void bno055_read_orientation(void)
{
    uint8_t raw_data[6];
    int ret = i2c_burst_read(i2c_dev, BNO055_I2C_ADDR, BNO055_EULER_H_LSB_ADDR, raw_data, 6);
    if (ret < 0) {
        printk("Failed to read orientation data\n");
        return;
    }

    // Combine MSB and LSB for each axis
    int16_t heading = (int16_t)((raw_data[1] << 8) | raw_data[0]);
    int16_t roll = (int16_t)((raw_data[3] << 8) | raw_data[2]);
    int16_t pitch = (int16_t)((raw_data[5] << 8) | raw_data[4]);

    // Scale to degrees (16 LSB per degree)
    double heading_deg = heading / 16.0;
    double roll_deg = roll / 16.0;
    double pitch_deg = pitch / 16.0;

    printk("Heading: %f deg, Roll: %f deg, Pitch: %f deg\n", heading_deg, roll_deg, pitch_deg);
}

int main(void)
{
    if (!device_is_ready(i2c_dev)) {
        printk("I2C device not ready\n");
        return;
    }

    printk("Initializing BNO055...\n");

    // Reset the sensor
    bno055_reset();

    // Set NDOF mode for absolute orientation
    if (bno055_set_mode(BNO055_OPR_MODE_NDOF) < 0) {
        return;
    }

    while (1) {
        // Read orientation (Euler angles)
        bno055_read_orientation();

        // Delay for readability
        k_sleep(K_MSEC(1000));
    }
}