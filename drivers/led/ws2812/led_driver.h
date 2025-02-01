#include <zephyr/drivers/led_strip.h>

int setup_led(void);
int set_all(struct led_rgb color);
int set_color(uint8_t index, struct led_rgb color);