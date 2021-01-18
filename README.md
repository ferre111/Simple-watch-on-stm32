# Simple smartwatch on STM32

The purpose of this project is to create a smartwatch equipped with an OLED display and a set of sensors. The fundamental hardware part of this watch will be a STM32F103 microcontroller based on one the Cortex-M3 core.

Device will be capable of measuring:
- time,
- temperature,
- air pressure,
- altitude,
- magnetic field,
- acceleration


Data from sensors and current time will be displayed on the OLED display equipped with the SSD1306 driver. All sensors and display are driven by I2C interface.

List of sensors:
- MPU6050 (GY-87)
- BMP180 (GY-87)
- QMC5883L (GY-87)
