# IMU-Driver and LED System
### Overview
Functions for the led strip exist in drivers/led/ws2812/*

Functions for the BNO055 exist in drivers/sensor/bno055/*

To use the led strip, run `setup_led()` after including `led/ws2812/led_driver.h`

To use the BNO055, run `setup_bno055()` after including `sensor/bno055/bno055.h`. This will also calibrate the sensor. 

`main` contains some sample code to run LEDs and BNO055 integrated together.

For background on communication protocols, please refer to these resources:
- [PROTOCOLS: UART - I2C - SPI - Serial communications #001](https://youtu.be/IyGwvGzrqp8?si=7uvTlQ6fvSE429jO)
- [Communication protocol in Embedded System | Synchronous & Asynchronous communication](https://youtu.be/bdgCFkc_RXY?si=EliNk__MKra6Cj4J)


### Implementation Details
The LEDs need 5V as input, but the nRF only outputs signals at 3.3V. Thus, we need a logic level converter. We currently have three logic converters, here are the notes for each:
- [AK-LVLSHF-TXB-BB](https://www.digikey.ca/en/products/detail/artekit-labs/AK-LVLSHF-TXB-BB/26606451)
- [TXS01018E 8CHANNEL LEVEL SHIFTER](https://www.digikey.ca/en/products/detail/sparkfun-electronics/19626/16570915)
- [STEMMA QT 3V TO 5V LEVEL BOOSTER](https://www.digikey.ca/en/products/detail/adafruit-industries-llc/5649/21283813?gclsrc=aw.ds&gad_source=1&gad_campaignid=20282404290&gbraid=0AAAAADrbLlg67z9-GvU_XGVB5ICFdALgx&gclid=Cj0KCQjw9czHBhCyARIsAFZlN8T_QAdYrXcRdhyFg7OYP3w6DAT9MsOGJQKRwBI2ConwPr-M8gSmfjMaAiTmEALw_wcB): This level shifter is used exclusively for stepping up i2c logic. The LEDs do not use i2c to communicate, instead they use a variation of SPI so we do not use this level shifter.


### Wiring Diagram
This diagram shows how to wire the LED strip, BNO055 and nRF52840dk together.
![Alt text](./images/IMU-LED%20connection.svg)