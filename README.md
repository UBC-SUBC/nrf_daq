This is the base repository for the firmware for the various sensors in the electrical system.

## Overview

The sensors inside the enclosures are:
- BNO055 IMU (I2C)
- BME280: Temperature Sensor (I2C)

The sensors that route to outside the enclosures are:
- MLX90393-SLQ-ABA-011-RE Hall Effect Sensor (both SPI and I2C possible)
- [Depth Sensor](https://bluerobotics.com/store/sensors-cameras/sensors/bar30-sensor-r1/)

Other things that interact with the nrf52840dk dev board
- WS2812B LED strip
- Switch to turn the power on and off

## Repository Setup Information
Each branch of the repository corresponds to a firmware project that is not yet completed. Please refer to the other branches for more detailed documentation. Open a pull-request for a branch to merge into main once the sub-project is complete. Here are the current branches:
- main: Current functioning code. Once a sub-project is completed, it is merged into this branch. This holds the code that will be ultimately run during competition. 
- LED_template: This branch contains a template for interfacing with the WS2812 LEDs. 
- hall-effect-i2c: This branch contains the code for the hall-effect sensor. Since the primary MCU must communicate with the hall effect MCU (see below), this also has the code that must be run on the HF-MCU for communication. 
- imu-driver: This branch contains the code relating to the IMU. The IMU displays its data using the LEDs, so the code for interfacing with the LEDs also lives on this branch. 
- SD-card: This branch contains the code for saving data to an SD card. Since it needs data to store, it also has the interface for the temperature sensor. 
- depth-sensor: This branch is for configuring the depth-sensor.
- flash-MCU: This branch is for flashing the code of a second MCU using the first one. 

## PCB Information
There are two boards, named Power Board and Control Board. The Power Board draws power from two inputs: USB-A is used to power the microcontroller and sensors, USB-C is used to power the LED lights. The microcontroller and all sensors run on 3.3V. The LED runs on 5V. There is a linear regulator to step the 3.3V up to 5V between the LED and the microcontroller.

### Hall Effect Sensor
The Hall Effect Sensor board has a microcontroller connected to a MLX90393-SLQ-ABA-011-RE. The microcontroller on the Hall Effect Sensor board communicates with the hall effect sensor using I2C. It can also be configured to use SPI. The two microcontrollers facilitate communication between the Hall Effect Sensor board and the Control Board using differential UART. Here are some resources:
- [Nordic Semiconductor UART documentation](https://docs.nordicsemi.com/bundle/ps_nrf52840/page/uart.html)
- [Differential I2C documentation](https://hackaday.com/2017/03/31/an-introduction-to-differential-i²c/)
- [This is the differential IC we are using](https://www.ti.com/product/SN65LVDS9638/part-details/SN65LVDS9638D)