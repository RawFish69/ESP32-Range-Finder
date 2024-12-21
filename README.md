# ESP32-Ranger-Finder
[![ESP32](https://img.shields.io/badge/ESP32-C3-green.svg)](https://www.espressif.com/en/products/socs/esp32-c3)
[![License](https://img.shields.io/badge/license-Open%20Source-blue.svg)](./LICENSE)
Handheld ranger finder with ESP32

## Overview
This project uses an ESP32 to measure distances via ultrasonic (HC-SR04) and ToF (VL53L1X) sensors.

## Requirements
- ESP32 or ESP32C3 board
- VL53L1X sensor
- HC-SR04 ultrasonic sensor
- Servo motor for scanning
- Arduino IDE or PlatformIO

## Installation
1. Clone or download this repository.
2. Open the project in your preferred IDE.
3. Install required libraries (Adafruit_VL53L1X, ArduinoJson, etc.).

## Usage
1. Upload the code to the ESP32.  
2. Connect to the “Range Finder Beta” Wi-Fi.  
3. Visit http://192.168.1.101 in a browser to view the radar UI and stats.

## Images
![Range Finder Prototype](misc/range_finder.png)
*ESP32 Range Finder Prototype*

## CAD Files
TODO

## Circuit Diagrams
TODO

## License
This project is available under an open-source license. See individual libraries for their licenses.
