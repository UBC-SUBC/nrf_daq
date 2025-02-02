#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>

#include "sensor/bno055/bno055.h"
#include "led/ws2812/led_driver.h"


int main(void)
{
    if(setup_led())
    {
        printk("Failed to setup LED\n");
        return -1;
    }

    if(setup_bno055())
    {
        printk("Failed to setup BNO055\n");
        return -1;
    }

    struct bno055_euler_t euler;
    
    while(1) {
        read_euler_hrp(&euler);
        if(euler.p > 100) {
            set_all(RGB(0, 50, 0));
        } else if (euler.p < -100) {
            set_all(RGB(50, 0, 0));
        } else {
            clear_all();
            printk("Cleared: %d\n", euler.p);
        }
        send_data();
        k_sleep(K_MSEC(500));
    }
}
