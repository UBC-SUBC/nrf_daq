#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>

#include "sensor/bno055/bno055.h"
#include "led/ws2812/led_driver.h"

#define LOG_LEVEL 4
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

struct bno055_t bno055;

int main(void)
{
    while(1)
    {
        if (setup_led() == 0)
        {
            printk("Failed to setup LED\n");
            return 1;
        }
        
        printk("LED setup successful\n");

        set_all(RGB(0, 0, 255));
        send_data();
    }
}
