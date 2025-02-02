# IMU-Driver and LED System

Functions for the led strip exist in drivers/led/ws2812/*
Functions for the BNO055 exist in drivers/sensor/bno055/*

To use the led strip, run `setup_led()` after including `led/ws2812/led_driver.h`
To use the BNO055, run `setup_bno055()` after including `sensor/bno055/bno055.h`. This will also calibrate the sensor. 

`main` contains some sample code to run LEDs and BNO055 integrated together.
