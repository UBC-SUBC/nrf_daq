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

    uint8_t reset[1] = {RST_REG};
    ret = i2c_write_dt(&dev_i2c, reset, sizeof(reset));
    k_usleep(SLEEP_TIME_US);
    if(ret != 0){
        printk("Reset failed to write to I2C device address %x\n", dev_i2c.addr);
        return -1;
    } else {
        printk("Reset set\n");
    }

    // set up exit
    uint8_t exit[1] = {0x80};
    ret = i2c_write_dt(&dev_i2c, exit, sizeof(exit));
    if(ret != 0){
        printk("Failed to write to I2C device address %x\n", dev_i2c.addr);
        return -1;
    } else {
        printk("Exit set\n");
    }

    k_usleep(SLEEP_TIME_US);

    



    while (1) {        
        uint8_t read_buf[8];
        uint8_t read_reg[1] = {RM_REG};
        ret = i2c_write_read_dt(&dev_i2c, read_reg, 2, read_buf, 8);
        if (ret != 0) {
			printk("Failed to write/read I2C device address %x at Reg. %x \n",
			       dev_i2c.addr, read_reg[0]);
		}

        uint8_t t = (read_buf[0] << 8) | read_buf[1];
        uint8_t x = (read_buf[2] << 8) | read_buf[3];
        uint8_t y = (read_buf[4] << 8) | read_buf[5];
        uint8_t z = (read_buf[6] << 8) | read_buf[7];

        //Print reading to console  
        printk("t: %d\n", t);
        printk("x: %d\n", x);
        printk("y: %d\n", y);
        printk("z: %d\n", z);

        k_usleep(SLEEP_TIME_US);
    }
}
