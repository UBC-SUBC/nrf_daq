#include <errno.h>
#include <string.h>

#define LOG_LEVEL 4
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(led_driver);

#include <zephyr/kernel.h>
#include <zephyr/drivers/led_strip.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/sys/util.h>

#include "led_driver.h"

#define STRIP_NODE		DT_ALIAS(led_strip)
#define STRIP_NUM_PIXELS	DT_PROP(DT_ALIAS(led_strip), chain_length)

#define RGB(_r, _g, _b) ((struct led_rgb){ .r = (_r), .g = (_g), .b = (_b) })

#define DELAY_TIME K_MSEC(50)

struct led_rgb pixels[STRIP_NUM_PIXELS];

static const struct device *const strip = DEVICE_DT_GET(STRIP_NODE);

int setup_led(void) 
{
    if (device_is_ready(strip)) {
		LOG_INF("Found LED strip device %s", strip->name);
	} else {
		LOG_ERR("LED strip device %s is not ready", strip->name);
		return 0;
	}

    return 1; // success
}

int set_all(struct led_rgb color) 
{
    for (size_t i = 0; i < STRIP_NUM_PIXELS; i++) {
		memcpy(&pixels[i], &color, sizeof(struct led_rgb));
    }

    return 1; // success
}

int set_color(uint8_t index, struct led_rgb color) 
{
    if(index >= STRIP_NUM_PIXELS) {
		return -EINVAL; // invalid index
	}

	memcpy(&pixels[index], &color, sizeof(struct led_rgb));

    return 1; // success
}

int send_data(void) 
{
	int rc = led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);

	if (rc) {
		LOG_ERR("couldn't update strip: %d", rc);
		return 0; // fail
	}

	return 1; // success
}