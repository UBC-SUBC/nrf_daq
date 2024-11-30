#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include "sensor/bno055/bno055.h"

#if !DT_HAS_COMPAT_STATUS_OKAY(bosch_bno055)
#error "No bosch,bno055 compatible node found in the device tree"
#endif


const struct device *bno055 = DEVICE_DT_GET_ANY(bosch_bno055);

int main(void)
{

        if (!device_is_ready(bno055))
        {
                return 1;
        }
        struct sensor_value accelx, accely, accelz;

        while (true)
        {
                sensor_sample_fetch(bno055);
                sensor_channel_get(bno055, SENSOR_CHAN_ACCEL_X, &accelx);
                sensor_channel_get(bno055, SENSOR_CHAN_ACCEL_Y, &accely);
                sensor_channel_get(bno055, SENSOR_CHAN_ACCEL_Z, &accelz);
                printk("accel x from sensor %f\n", sensor_value_to_double(&accelx));
                printk("accel y from sensor %f\n", sensor_value_to_double(&accely));
                printk("accel z from sensor %f\n", sensor_value_to_double(&accelz));
                k_sleep(K_MSEC(1000));
        }
        

        return 0;
}
