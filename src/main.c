/*
* Copyright (c) 2016 Intel Corporation
*
* SPDX-License-Identifier: Apache-2.0
*/

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>

#include "reg.h"

#define I2C0_NODE DT_NODELABEL(mlx_90393)

typedef enum mlx90393_resolution {
    MLX90393_RES_16,
    MLX90393_RES_17,
    MLX90393_RES_18,
    MLX90393_RES_19,
} mlx90393_resolution_t;

int main(void)
{

    int ret;
    uint8_t ret_val[1] = {};

    printk("Hall effect sensor application\n");
    static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NODE);

    if (!device_is_ready(dev_i2c.bus)) {
        printk("I2C device not ready\n");
        return -1;
    } else {
        printk("I2C device ready\n");
        printk("I2C bus: %s, Address: 0x%X\n", dev_i2c.bus->name, dev_i2c.addr);
    }

    k_usleep(SLEEP_TIME_US);

    // set up exit
    uint8_t exit[1] = {EX_REG};
    ret = i2c_write_read_dt(&dev_i2c, exit, sizeof(exit), ret_val, sizeof(ret_val));
    if(ret != 0){
        printk("Failed to write to I2C device address %x\n", dev_i2c.addr);
        return -1;
    } else {
        printk("Exit set, status byte: %d\n", ret_val[0]);
    }

    k_usleep(SLEEP_TIME_US);

    // set up reset
    uint8_t reset[1] = {RST_REG};
    ret = i2c_write_read_dt(&dev_i2c, reset, sizeof(reset), ret_val, sizeof(ret_val));
    if(ret != 0){
        printk("Reset failed to write to I2C device address %x\n", dev_i2c.addr);
        return -1;
    } else {
        printk("Reset set, status byte: %d\n", ret_val[0]);
    }

    k_usleep(SLEEP_TIME_US);

    // set up gain
    uint8_t gain[2] = {READ_CMD, GAIN_REG};
    uint8_t gain_result[3] = {};
    ret = i2c_write_read_dt(&dev_i2c, gain, sizeof(gain), gain_result, sizeof(gain_result));
    if(ret != 0){
        printk("Failed to write/read I2C device address %x at Reg. %x \n",
            dev_i2c.addr, gain[0]);
        return -1;
    } else {
        printk("Read gain, status byte: %d\n", gain_result[0]);
    }

    gain_result[2] |= 0x03;

    uint8_t write[4] = {WRITE_CMD, gain_result[1], gain_result[2], GAIN_REG << 2};
    ret = i2c_write_read_dt(&dev_i2c, write, sizeof(write), ret_val, sizeof(ret_val));
    if(ret != 0){
        printk("Failed to write I2C device address %x\n", dev_i2c.addr);
        return -1;
    } else {
        printk("Gain set, status byte: %d\n", ret_val[0]);
    }

    // set up resolution
    uint8_t resolution_settings[2] = {READ_CMD, RES_REG << 2};
    uint8_t res[3] = {};
    ret = i2c_write_read_dt(&dev_i2c, resolution_settings, sizeof(resolution_settings), res, sizeof(res));
    if(ret != 0){
        printk("Failed to write/read I2C device address %x at Reg. %x \n",
            dev_i2c.addr, res[0]);
        return -1;
    } else {
        printk("Read resolution, status byte: %d\n", res[0]);
    }
    uint16_t data = (res[1] << 8) | res[2];
    mlx90393_resolution_t resolution = MLX90393_RES_16;

    data &= ~0x0060;
    data |= resolution << 5; // set to 16 bit resolution
    data &= ~0x0180;
    data |= resolution << 7; // set to 16 bit resolution
    data &= ~0x0600;
    data |= resolution << 9; // set to 16 bit resolution

    uint8_t write_resolution[4] = {WRITE_CMD, data >> 8, data & 0xFF, RES_REG << 2};
    ret = i2c_write_read_dt(&dev_i2c, write_resolution, sizeof(write_resolution), ret_val, sizeof(ret_val));
    if(ret != 0){
        printk("Failed to write I2C device address %x\n", dev_i2c.addr);
        return -1;
    } else {
        printk("Resolution set, status byte: %d\n", ret_val[0]);
    }

    while (1) {        
        // single measurement command
        uint8_t sm[1] = {SM_REG};
        ret = i2c_write_read_dt(&dev_i2c, sm, sizeof(sm), ret_val, sizeof(ret_val));
        if(ret != 0){
            printk("Failed to write/read I2C device address %x at Reg. %x \n",
                dev_i2c.addr, sm[0]);
            return -1;
        } else {
            printk("Single measurement set, status byte: %d\n", ret_val[0]);
        }

        k_msleep(SLEEP_TIME_US);

        uint8_t read_buf[9];
        uint8_t read_reg[1] = {RM_REG};
        ret = i2c_write_read_dt(&dev_i2c, read_reg, sizeof(read_reg), read_buf, sizeof(read_buf));
        if (ret != 0) {
			printk("Failed to write/read I2C device address %x at Reg. %x \n",
			       dev_i2c.addr, read_reg[0]);
		}

        int16_t t = (read_buf[1] << 8) | read_buf[2];
        int16_t x = (read_buf[3] << 8) | read_buf[4];
        int16_t y = (read_buf[5] << 8) | read_buf[6];
        int16_t z = (read_buf[7] << 8) | read_buf[8];

        //Print reading to console  
        printk("Status byte: %d\n", read_buf[0]);
        printk("t: %d\n", t);
        printk("x: %d\n", x);
        printk("y: %d\n", y);
        printk("z: %d\n\n", z);
        
        if(read_buf[0] == 17 || read_buf[0] == 16) {
            return -1;
        }

        k_msleep(SLEEP_TIME_US);
    }
}
