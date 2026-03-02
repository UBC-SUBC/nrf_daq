#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>

// Depth sensor additions
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>

#include "sensor/bno055/bno055.h"
#include "led/ws2812/led_driver.h"
#include "bar30/bar30.h"

#define MOVING_AVERAGE_WINDOW 10  // Samples for smoothing

// Constants for pressure to depth conversion
#define GRAV 9.80665
#define WATER_DENSITY 997.0474
#define ATM_PRESSURE 101500

// Reference to sensor in overlay file
#define I2C_NODE DT_NODELABEL(mysensor)

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
    static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_NODE);

    // Check if i2c communication is ready for depth-sensor
    if (!device_is_ready(dev_i2c.bus)) {
        printk("I2C bus not ready\n");
        return -1;
    }
  
    // Check if depth-sensor is initialized and ready
    if (bar30_init(&dev_i2c) != 0) {
        printk("Initialization failed\n");
        return -1;
    }
  
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
      
        int32_t pressure_pa;

        if (bar30_read_pressure_pa(&dev_i2c, &pressure_pa) == 0) {
            double depth = (pressure_pa - ATM_PRESSURE) / (GRAV * WATER_DENSITY);

            printk("Pressure: %d Pa\n", pressure_pa);
            printk("Depth: %.2lf m\n", depth);
        } else {
            printk("Read failed\n");
        }
      
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



