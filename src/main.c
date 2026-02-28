#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>

#include "sensor/bno055/bno055.h"
#include "led/ws2812/led_driver.h"


#define MOVING_AVERAGE_WINDOW 10  // Samples for smoothing

int pitch_history[MOVING_AVERAGE_WINDOW] = {0}; 
int history_index = 0;

int calculate_moving_average() {
    int sum = 0;
    for (int i = 0; i < MOVING_AVERAGE_WINDOW; i++) {
        sum += pitch_history[i];
    }
    return sum / MOVING_AVERAGE_WINDOW;
}

int main(void)
{
    if(setup_led()) {
        printk("Failed to setup LED\n");
        return -1;
    }

    if(setup_bno055()) {
        printk("Failed to setup BNO055\n");
        return -1;
    }


    struct bno055_euler_t euler;

    while(1) {
        read_euler_hrp(&euler);

        // If values are too big, assume centidegrees
        int corrected_pitch = euler.p;
        if (corrected_pitch > 1000 || corrected_pitch < -1000) {
            corrected_pitch /= 100;  // Convert from centidegrees to degrees
        }

        // Clamp pitch to prevent out-of-bounds errors
        if (corrected_pitch > 180) corrected_pitch = 180;
        if (corrected_pitch < -180) corrected_pitch = -180;

        printk("Raw Pitch= %d, Corrected= %d\n", euler.p, corrected_pitch);

        // Store new value in moving average buffer
        pitch_history[history_index] = corrected_pitch;
        history_index = (history_index + 1) % MOVING_AVERAGE_WINDOW;

        // Compute moving average
        int smoothed_pitch = calculate_moving_average();

        // Map smoothed pitch to LED index
        int led_index = (((smoothed_pitch + 180) * STRIP_NUM_PIXELS) / 360)+ STRIP_NUM_PIXELS/2;

        if (led_index < 0) led_index = 0;
        if (led_index >= STRIP_NUM_PIXELS) led_index = STRIP_NUM_PIXELS - 1;

        clear_all();

        if(smoothed_pitch > 5) {
            set_color(led_index, RGB(0, 50, 0));  // Green
            printk("led_ind = %d, colr = G\n", led_index);
            set_color(led_index + 1, RGB(0, 50, 0));  // Red
        
        } else if (smoothed_pitch < -5) {
            set_color(led_index, RGB(50, 0, 0));  // Red
            set_color(led_index + 1, RGB(50, 0, 0));  // Red
            printk("led_ind = %d, colr = R\n", led_index);
        
        } else {
            clear_all();  // Neutral
            printk("led_ind = --, colr = --\n");
        }

        send_data();
        k_sleep(K_MSEC(250));
    }
}



