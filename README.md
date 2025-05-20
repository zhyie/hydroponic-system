# Arduino Hydroponic System

An Arduino-based hydroponic system that monitors parameters and regulates these parameter to their ideal range.

## Project Overview

This project implements a hydroponic growing system controlled by an Arduino Mega microcontroller. It continuously monitors key parameters such as water temperature, pH, electrical conductivity (EC), and total dissolved solids (TDS), while automating processes including water cooling, pH dosing, and nutrient delivery through fertigation. The goal is to simplify hydroponics for urban gardening and reduce water wastage compared to traditional agriculture.

## Features

- Real-time Monitoring
- Automated Water Cooling
- Automated pH Dosing
- Automated Nutrient Delivery (Fertigation)
- User-configurable Thresholds
- Data Logging and Display

## Hardware Used

- Arduino Mega
- DS18B20 Temperature sensor
- DFRobot Gravity pH sensor
- DFRobot Gravity EC sensor
- DFRobot Gravity TDS sensor
- DC-DC Buck Converter
- Relay module
- MOSFET (IRLZ44N)
- Peristaltic pump
- Thermoelectric
- Power Supply

## Software and Libraries

- Arduino IDE
- OneWire and DallasTemperature (for DS18B20)
- DFRobot Gravity Libraries (for pH, EC, TDS)
- Nextion Editor
- Arduino Nextion Library

## Installation and Setup

1. Clone this repository:
```
git clone https://github.com/zhyie/hydroeg.git
```
2. Open the Arduino project file (`sketch/sketch.ino`) in the Arduino IDE
3. Install the required libraries (see `docs/libraries.txt`)
4. Connect the sensors and actuators to the Arduino (see `docs/circuit_diagram.png`)
5. Upload Nextion HMI file if using Nextion display (`sketch/display.HMI`)
6. Upload the code to your Arduino board

## Usage

- The system automatically monitors pH, temperature, EC, and TDS.
- The system automatically controls water cooling, pH dosing, and nutrient delivery based on preset thresholds.
- View real-time sensor data on the Nextion display or Arduino serial monitor.
- Use the Nextion interface to manually control parameters if needed.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE.md) file for details.
