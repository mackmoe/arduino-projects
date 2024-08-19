---

# Water Flow Sensor Arduino Project

This project is designed to monitor water flow using an Arduino and a water flow sensor. It can be used to measure the flow rate, total water usage, and more, making it ideal for various applications such as irrigation systems, water conservation, and monitoring household water usage.

## Table of Contents

- [Introduction](#introduction)
- [Components Required](#components-required)
- [Circuit Diagram](#circuit-diagram)
- [Arduino Code](#arduino-code)
- [Installation](#installation)
- [Usage](#usage)
- [Troubleshooting](#troubleshooting)
- [Future Improvements](#future-improvements)
- [License](#license)

## Introduction

The water flow sensor works by measuring the rate of water passing through the sensor. This project uses an Arduino to read the sensor data and calculate the flow rate and total volume of water used. The data can be displayed on an LCD or sent to a computer via serial communication for further analysis.

## Components Required

- Arduino (e.g., Uno, Nano, etc.)
- Water Flow Sensor Model FL-S401A 
- Power Supply (e.g., 9V battery or USB power)

## Circuit Diagram

Below is the basic circuit diagram for connecting the water flow sensor to the Arduino:

```
+-------------------+
| Water Flow Sensor  |
|                   |
|    VCC (Red) ----> +5V on Arduino
|    GND (Black) ---> GND on Arduino
|    Signal (Yellow) -> Digital Pin 2 on Arduino
+-------------------+
```

## Installation

1. Clone this repository or download the ZIP file and extract it.
2. Connect the components as described in the circuit diagram.
3. Upload the provided Arduino code to your Arduino board using the Arduino IDE.
4. Open the Serial Monitor to view the flow rate or observe it on the LCD.

## Usage

- Ensure that the water flow sensor model is the same one here and is properly connected to the water supply.
- Start the Arduino, and the flow rate should be displayed on the LCD or Serial Monitor.
- Use the data to monitor water usage, automate systems, or trigger alerts.
- The sensor has a 7mm coupling on both sides and is therefore easy to connect to a 6mm hose.
- The output of the sensor gives 98 pulses per second with a duty cycle of approximately 50% for each liter of fluid passing through per minute: Q [L/min] = fpulse [Hz]/98.

### [Specifications for water flow sensor model FL-S401A](https://www.tinytronics.nl/en/sensors/liquid/yf-s401-water-flow-sensor)

- Voltage range: 5-24V
- Pulse frequency per L/min: 98Hz
- Measuring range: 0.3-6L/min (with an accuracy of +-10%)
- Maximum water pressure: 8bar
- Working temperature: -25-80°C
- Duty cycle pulse: 50% +-10%
- Voltage pulse (with 5V as input voltage): 4.7V
- Adjust the piece of code from `float calibrationFactor = 4.5;` to `float calibrationFactor = 98;` 

## Troubleshooting

- **Incorrect flow rate:** Verify that the sensor is properly calibrated and connected.
- **No data on Serial Monitor:** Ensure that the correct COM port is selected and the baud rate is set to 9600.

## Future Improvements

- Add a data logger to save water usage over time.
- Daemonize a Serial Plotter.
- Implement wireless communication to send data to a remote server.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---