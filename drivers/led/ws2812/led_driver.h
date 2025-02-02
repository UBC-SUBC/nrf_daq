#include <zephyr/drivers/led_strip.h>

#define RGB(_r, _g, _b) ((struct led_rgb){ .r = (_r), .g = (_g), .b = (_b) })

int setup_led(void);
int set_all(struct led_rgb color);
int set_color(uint8_t index, struct led_rgb color);
int send_data(void);
int clear_all(void);