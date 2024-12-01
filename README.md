
# WS2812 LED Strip Setup with nRF52840

This guide walks you through setting up a WS2812 LED strip with the nRF52840 microcontroller. It includes hardware connections, software setup, and example code to control the LEDs.

---

## Table of Contents

1. [About the Project](#about-the-project)
2. [Hardware Requirements](#hardware-requirements)
3. [Software Requirements](#software-requirements)
4. [Hardware Connections](#hardware-connections)
5. [Software Setup](#software-setup)
6. [Example Code](#example-code)
7. [Usage](#usage)
8. [Troubleshooting](#troubleshooting)
9. [Acknowledgements](#acknowledgements)

---

## About the Project

The WS2812 is a popular individually-addressable RGB LED strip that uses a single data pin for communication. The nRF52840, a powerful ARM Cortex-M4F microcontroller, can generate the precise timing signals required to control these LEDs. 

This project demonstrates how to:
- Connect and power a WS2812 LED strip.
- Generate the necessary timing signals.
- Create dynamic lighting patterns using the nRF52840.

---

## Hardware Requirements

- **nRF52840 Development Kit (DK)** or compatible board.
- **WS2812 LED Strip** (ensure you know the number of LEDs in the strip).
- **5V Power Supply** (capable of handling the current needs of your LED strip, typically 60mA per LED at full brightness).
- **Logic Level Shifter** (optional, if your WS2812 requires a 5V data signal).

---

## Software Requirements

- [nRF Connect SDK](https://developer.nordicsemi.com/nRF_Connect_SDK/).
- Zephyr RTOS (included with the nRF Connect SDK).
- Segger Embedded Studio or other supported IDEs for development.
- Python or Node.js (optional, for testing patterns via serial interface).

---

## Hardware Connections

1. **Powering the LEDs**:
   - Connect the **5V** pin of the WS2812 strip to a 5V power supply.
   - Connect the **GND** pin of the WS2812 strip to the power supply ground and the nRF52840 ground.

2. **Data Line**:
   - Connect the nRF52840 GPIO pin **P0.29** and the **DIN** pin of the WS2812 strip.
   - If the WS2812 strip requires 5V logic, use a **logic level shifter** to step up the GPIO signal from 3.3V to 5V.
   - Pins **P0.31** and **P0.02** are occupied for the driving SPI peripheral.

---

## Troubleshooting

- **LEDs not lighting up**:
  - Check the power connections.
  - Ensure the data line uses the correct logic level.
- **Incorrect colors**:
  - Verify the RGB order (e.g., GRB vs RGB).
- **Timing issues**:
  - Adjust PWM timings based on WS2812 datasheet specifications.
- **Power Requirement**
  - Insure you have a 5V supply that can support the current for each LED.

---

## Acknowledgements

- [Nordic Semiconductor](https://www.nordicsemi.com/) for the nRF52840 platform.
- [Zephyr RTOS](https://zephyrproject.org/) for the real-time operating system.
- [WS2812 LED Datasheet](https://cdn-shop.adafruit.com/datasheets/WS2812.pdf).
- [ narvalotechmbolivar-nordic and mbolivar-nordic] (https://github.com/nrfconnect/sdk-zephyr/tree/v3.3.99-ncs1-1/samples/drivers/led_ws2812/boards)
