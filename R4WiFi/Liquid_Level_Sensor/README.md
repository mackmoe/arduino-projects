---

# Liquid Level Sensor Arduino R4 WiFi Project

This project is designed to detect water level in a container using an Arduino R4 WiFi and a water level sensor. It can be used to measure the flow rate, total water usage, and more, making it ideal for various applications such as irrigation systems, water conservation, and monitoring household water usage.

## Table of Contents

- [Introduction](#introduction)
- [Components Required](#components-required)
- [Circuit Diagram](#circuit-diagram)
- [Arduino R4 WiFi Code](#Arduino R4 WiFi-code)
- [Installation](#installation)
- [Usage](#usage)
- [Troubleshooting](#troubleshooting)
- [Future Improvements](#future-improvements)
- [License](#license)

## Introduction

Use the Arduino R4 WiFi IDE software to upload the test code on the Arduino R4 WiFi board, connect the line according to the wiring method, use the USB line power, dial the SET dial switch up, dial the right dial switch to 3V, open the serial port monitor, set the wave. The special rate is 9600. When the sensor senses the liquid, the D13 indicator on the Arduino R4 WiFi board lights up, and the serial monitor displays 1 as shown below, otherwise the D13 indicator on the Arduino R4 WiFi board goes out and 0 is displayed on the serial monitor.

## Components Required

- Arduino R4 WiFi (e.g., Uno, Nano, etc.)
- Non-Contact Water/Liquid Level Sensor SKU: CQRSENYW001 
- Power Supply (e.g., 9V battery or USB power)

## Circuit Diagram

Below is the basic circuit diagram for connecting the water level sensor to the Arduino R4 WiFi:

```
+-------------------+
| Water Level Sensor  |
|                   |
|    VCC (Red) ----> +3V on Arduino R4 WiFi
|    GND (Black) ---> GND on Arduino R4 WiFi
|    Signal (Green) -> Digital Pin 7 on Arduino R4 WiFi
+-------------------+
```

## Installation

1. Clone this repository or download the ZIP file and extract it.
2. Connect the components as described in the circuit diagram.
3. Upload the provided Arduino R4 WiFi code to your Arduino R4 WiFi board using the Arduino R4 WiFi IDE.
4. Open the Serial Monitor to view the flow rate or observe it on the LCD.

## Usage

- Ensure that the water level sensor model is the same one here and is properly connected to the water supply.
- Start the Arduino R4 WiFi


### [Specifications for water level sensor SKU: CQRSENYW001](http://www.cqrobot.wiki/index.php/Non-Contact_Water/Liquid_Level_Sensor_SKU:_CQRSENYW001)

**Sensor Probe and Adapter Specifications**

- Input Voltage: DC 5V
- Current Consumption: 5mA
- Output Voltage (High Level): 3.3V or 5V (Right Dial Switch Control)
- Output Voltage: 0
- Output Current: 1mA to 50mA
- Response Time: 500ms
- Working Environment Temperature: 0 to 105 Degree Celsius
- Induction Thickness (Sensitivity) Range: 0 to 13 mm
- Humidity: 5% to 100%
- Material: ABS
- Waterproof Performance: IP67
- Dimension: 31.6mm * 30mm
- Mounting hole size: 3.0mm

**Ocean interface Cable Specifications**

- Cable Specifications: 22AWG
- Material: Silicone
- Length: 21cm
- Withstand Voltage: Less Than 50V
- Withstand Current: Less Than 1000MA
- Line Sequence: Black-Negative Power Supply, Red-Positive Power Supply, Green-Signal.


## Troubleshooting

- **No data on Serial Monitor:** Ensure that the correct COM port is selected and the baud rate is set to 9600.

## Future Improvements

- Add something

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---