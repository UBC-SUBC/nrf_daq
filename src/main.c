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

#include <zephyr/drivers/uart.h>

#include "reg.h"

#define SLEEP_TIME_MS 5000
#define RECEIVE_BUFF_SIZE 10
#define RECEIVE_TIMEOUT 100

#define I2C0_NODE DT_NODELABEL(mlx_90393)

static uint8_t rx_buf[10] = {0}; //A buffer to store incoming UART data 
static uint8_t tx_buf[] =  {"Host: B\n\r"}; //send buffer

const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart1));

// static uint8_t line_buf[64];
// static size_t line_pos = 0;

static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
	switch (evt->type) {
	
	case UART_TX_DONE:
		printk("Sent data successfully\n");
		break;

	case UART_TX_ABORTED:
		// do something
		break;
		
	case UART_RX_RDY:
        //printk("Received: %.*s\n", sizeof(rx_buf), rx_buf);

        break;

	case UART_RX_BUF_REQUEST:
		// do something
		break;

	case UART_RX_BUF_RELEASED:
        
        int16_t x = (rx_buf[1] << 8 | rx_buf[2]);
        int16_t y = (rx_buf[3] << 8 | rx_buf[4]);
        int16_t z = (rx_buf[5] << 8 | rx_buf[6]);

        printk("Status byte %d\n", read_buf[0]);
        printk("x: %d\n", x);
        printk("y: %d\n", y);
        printk("z: %d\n", z);
		// do something
		break;

	case UART_RX_STOPPED:
    //printk("Full Received: %.*s\n", sizeof(rx_buf), rx_buf);
		// do something
		break;
    
    case UART_RX_DISABLED:
        
        //printk("Received: %.*s\n", sizeof(rx_buf), rx_buf); 
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

    while(1) {
        k_msleep(SLEEP_TIME_MS);
        ret = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);
        if (ret) {
            return ret;
        }
    }
}
