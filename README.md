This is the base repository for the firmware for the various sensors in the electrical system.

## Overview

The sensors inside the enclosures are:
- BNO055 IMU (I2C)

The sensors that route to outside the enclosures are:
- MLX90393-SLQ-ABA-011-RE Hall Effect Sensor (both SPI and I2C possible)
- [Depth Sensor](https://bluerobotics.com/store/sensors-cameras/sensors/bar30-sensor-r1/)

Other things that interact with the nrf52840dk dev board
- WS2812B LED strip
- Switch to turn the power on and off

## PCB Information
There are two boards, named Power Board and Control Board. The Power Board draws power from two inputs: USB-A is used to power the microcontroller and sensors, USB-C is used to power the LED lights. The microcontroller and all sensors run on 3.3V. The LED runs on 5V. There is a linear regulator to step the 3.3V up to 5V between the LED and the microcontroller.

### Hall Effect Sensor
The Hall Effect Sensor board has a microcontroller connected to a MLX90393-SLQ-ABA-011-RE. The microcontroller on the Hall Effect Sensor board communicates with the hall effect sensor using I2C. It can also be configured to use SPI. The two microcontrollers facilitate communication between the Hall Effect Sensor board and the Control Board using differential UART. Here are some resources:
- [Nordic Semiconductor UART documentation](https://docs.nordicsemi.com/bundle/ps_nrf52840/page/uart.html)
- [Differential I2C documentation](https://hackaday.com/2017/03/31/an-introduction-to-differential-i²c/)
- [This is the differential IC we are using](https://www.ti.com/product/SN65LVDS9638/part-details/SN65LVDS9638D)
