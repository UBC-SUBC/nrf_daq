/*
* Copyright (c) 2016 Intel Corporation
*
* SPDX-License-Identifier: Apache-2.0
*/

#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>

#include <zephyr/drivers/uart.h>

#include "reg.h"

#define SLEEP_TIME_MS 10000
#define RECEIVE_BUFF_SIZE 10
#define RECEIVE_TIMEOUT 100
#define SENSOR_THRESHOLD 500

#define I2C0_NODE DT_NODELABEL(mlx_90393)

static uint8_t rx_buf[10] = {0}; //A buffer to store incoming UART data 
static uint8_t tx_buf[] =  {"Host: B\n\r"}; //send buffer

const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart1));

typedef enum mlx90393_resolution {
    MLX90393_RES_16,
    MLX90393_RES_17,
    MLX90393_RES_18,
    MLX90393_RES_19,
} mlx90393_resolution_t;

static uint32_t detect_rpm_from_reading(int16_t sample)
{
    static bool was_above_threshold;
    static uint32_t last_edge_ms;

    bool above = sample > SENSOR_THRESHOLD;
    uint32_t rpm = 0U;

    if (!was_above_threshold && above) {
        uint32_t now_ms = k_uptime_get_32(); // get time in ms since program started
        if (last_edge_ms != 0U) {
            uint32_t period_ms = now_ms - last_edge_ms;
            if (period_ms > 0U) {
                rpm = 60000U / period_ms;
            }
        }
        last_edge_ms = now_ms;
    }

    was_above_threshold = above;
    return rpm;
}

static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
	switch (evt->type) {
	
	case UART_TX_DONE:
		printk("Sent data successfully\n");
		break;

	case UART_TX_ABORTED:
		printk("Transmission aborted due to timeout\n");
		break;
		
	case UART_RX_RDY:
        printk("Received: %.*s\n", sizeof(rx_buf), rx_buf); 
        uint16_t x = (rx_buf[1] << 8 | rx_buf[2]);
        uint16_t y = (rx_buf[3] << 8 | rx_buf[4]);
        uint16_t z = (rx_buf[5] << 8 | rx_buf[6]);

        printk("YES Status byte %d\n", rx_buf[0]);
        printk("x: %d\n", x);
        printk("y: %d\n", y);
        printk("z: %d\n", z);
        break;

	case UART_RX_BUF_RELEASED:
		break;

	case UART_RX_STOPPED:
    //printk("Full Received: %.*s\n", sizeof(rx_buf), rx_buf);
		// do something
		break;
    
    case UART_RX_DISABLED:
        uart_rx_enable(dev, rx_buf, sizeof(rx_buf), 100);
        break;
		
	default:
		break;
	}
}

int main(void)
{
    int ret;
    
    if (!device_is_ready(uart)) {
        printk("Error: device is not ready\n");
        return -1;
    }

    const struct uart_config uart_cfg = {
		.baudrate = 115200,
		.parity = UART_CFG_PARITY_NONE,
		.stop_bits = UART_CFG_STOP_BITS_1,
		.data_bits = UART_CFG_DATA_BITS_8,
		.flow_ctrl = UART_CFG_FLOW_CTRL_NONE
	};

    ret = uart_configure(uart, &uart_cfg);
	if (ret == -ENOSYS) {
		return -ENOSYS;
	}

    /* start here */
    ret = uart_callback_set(uart, uart_cb, NULL);
    if (ret) {
        return ret;
    }

    /* receive data */
    ret = uart_rx_enable(uart, rx_buf, sizeof(rx_buf), 100);
    if (ret) {
        return ret;
    }

    /* transfer data */
    ret = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);
	if (ret) {
		return ret;
	}

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

    const int calibration_samples = 8;
    int32_t x_offset = 0;
    int32_t y_offset = 0;
    int32_t z_offset = 0;

    for (int i = 0; i < calibration_samples; ++i) {
        uint8_t sm[1] = {SM_REG};
        ret = i2c_write_read_dt(&dev_i2c, sm, sizeof(sm), ret_val, sizeof(ret_val));
        if (ret != 0) {
            printk("Failed to write/read I2C device address %x at Reg. %x \n",
                dev_i2c.addr, sm[0]);
            return -1;
        }

        k_usleep(SLEEP_TIME_US);

        uint8_t read_buf[7];
        uint8_t read_reg[1] = {RM_REG};
        ret = i2c_write_read_dt(&dev_i2c, read_reg, sizeof(read_reg), read_buf, sizeof(read_buf));
        if (ret != 0) {
			printk("Failed to write/read I2C device address %x at Reg. %x \n",
			       dev_i2c.addr, read_reg[0]);
            return -1;
		}

        x_offset += (int16_t)((read_buf[1] << 8) | read_buf[2]);
        y_offset += (int16_t)((read_buf[3] << 8) | read_buf[4]);
        z_offset += (int16_t)((read_buf[5] << 8) | read_buf[6]);

        k_usleep(SLEEP_TIME_US);
    }

    x_offset /= calibration_samples;
    y_offset /= calibration_samples;
    z_offset /= calibration_samples;

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

        k_usleep(SLEEP_TIME_US);

        uint8_t read_buf[7];
        uint8_t read_reg[1] = {RM_REG};
        ret = i2c_write_read_dt(&dev_i2c, read_reg, sizeof(read_reg), read_buf, sizeof(read_buf));
        if (ret != 0) {
			printk("Failed to write/read I2C device address %x at Reg. %x \n",
			       dev_i2c.addr, read_reg[0]);
		}

        int16_t x = ((read_buf[1] << 8) | read_buf[2]) - (int16_t)x_offset;
        int16_t y = ((read_buf[3] << 8) | read_buf[4]) - (int16_t)y_offset;
        int16_t z = ((read_buf[5] << 8) | read_buf[6]) - (int16_t)z_offset;

        //Print reading to console  
        // printk("Status byte: %d\n", read_buf[0]);
        // printk("x: %d\n", x);
        // printk("y: %d\n", y);
        // printk("z: %d\n\n", z);

        uint32_t rpm = detect_rpm_from_reading(z);
        if (rpm > 0U) {
            printk("RPM: %u\n", rpm);
        }

        ret = uart_tx(uart, read_buf, sizeof(read_buf), SYS_FOREVER_US);
        if (ret) {
            return ret;
        }

        k_msleep(SLEEP_TIME_US);
    }
}
