# Multi-Sensor Monitoring with ESP-IDF and FreeRTOS

## Description

This project demonstrates the use of **ESP-IDF** and **FreeRTOS** for multitasking and sensor management on an ESP32 microcontroller. It integrates three sensors **BMP280 (temperature sensor)**, **KY-003 (magnetic sensor)**, and an **ADC-based soil moisture sensor** to monitor environmental conditions and log data efficiently. Each sensor operates as an independent FreeRTOS task, showcasing real-time multitasking in resource-constrained environments.

## Features

- **BMP280 Temperature Monitoring**  
   Reads temperature data using the BMP280 sensor via I2C, calibrates raw sensor data, and logs temperature in °C.

- **KY-003 Magnetic Field Detection**  
   Detects the presence or absence of a magnetic field and logs the state.

- **Soil Moisture Level Monitoring**  
   Measures soil moisture using an ADC input and maps the raw ADC value to a percentage.

## Hardware Requirements

- **ESP32 Development Board**
- **BMP280 Sensor** (I2C connection)
- **KY-003 Magnetic Sensor**
- **Soil Moisture Sensor** (ADC input)

## Pin Configuration

| Peripheral       | GPIO Pin          |
|------------------|-------------------|
| BMP280 I2C SDA   | GPIO 19           |
| BMP280 I2C SCL   | GPIO 20           |
| KY-003 Sensor    | GPIO 6            |
| Soil Sensor ADC  | ADC Channel 2 (GPIO 2) |

## Software Requirements

- **ESP-IDF** (Espressif IoT Development Framework)
- **FreeRTOS** (Built into ESP-IDF)

## How to Use

1. **Clone the Repository**  
   Clone the repository to your local machine and navigate to the project directory.

2. **Setup ESP-IDF**  
   Follow the [ESP-IDF setup guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) to configure your development environment.

3. **Build and Flash**  
   Run the following commands:
   ```bash
   idf.py build
   idf.py flash
    ```
4. **Monitor Logs**
    ```bash
    idf.py monitor
    ```
