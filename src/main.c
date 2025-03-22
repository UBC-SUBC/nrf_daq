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
        printk("Exit set\n");
    }

    k_usleep(SLEEP_TIME_US);

    // set up reset
    uint8_t reset[1] = {RST_REG};
    ret = i2c_write_read_dt(&dev_i2c, reset, sizeof(reset), ret_val, sizeof(ret_val));
    if(ret != 0){
        printk("Reset failed to write to I2C device address %x\n", dev_i2c.addr);
        return -1;
    } else {
        printk("Reset set\n");
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
        printk("Read gain\n");
    }

    gain_result[2] |= 0x01;

    uint8_t write[4] = {WRITE_CMD, gain_result[1], gain_result[2], GAIN_REG << 2};
    ret = i2c_write_read_dt(&dev_i2c, write, sizeof(write), ret_val, sizeof(ret_val));
    if(ret != 0){
        printk("Failed to write I2C device address %x\n", dev_i2c.addr);
        return -1;
    } else {
        printk("Gain set\n");
    }

    while (1) {        
        uint8_t read_buf[9];
        uint8_t read_reg[1] = {RM_REG};
        ret = i2c_write_read_dt(&dev_i2c, read_reg, sizeof(read_reg), read_buf, sizeof(read_buf));
        if (ret != 0) {
			printk("Failed to write/read I2C device address %x at Reg. %x \n",
			       dev_i2c.addr, read_reg[0]);
		}

        uint8_t t = (read_buf[1] << 8) | read_buf[2];
        uint8_t x = (read_buf[3] << 8) | read_buf[4];
        uint8_t y = (read_buf[5] << 8) | read_buf[6];
        uint8_t z = (read_buf[7] << 8) | read_buf[8];

        //Print reading to console  
        printk("t: %d\n", t);
        printk("x: %d\n", x);
        printk("y: %d\n", y);
        printk("z: %d\n\n", z);

        k_msleep(SLEEP_TIME_US);
    }
}
