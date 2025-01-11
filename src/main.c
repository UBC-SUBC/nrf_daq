#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include "sensor/bno055/bno055.h"
#include "sensor/bno055/driver_functions.h"

#if !DT_HAS_COMPAT_STATUS_OKAY(bosch_bno055)
#error "No bosch,bno055 compatible node found in the device tree"
#endif


int main(void)
{
        s32 comres = bno055_data_readout_template();
        printk("return value: %d\n", comres);

        return 0;
}
