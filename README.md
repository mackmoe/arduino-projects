# Arduino Projects

A repository of Arduino-based sensor monitoring scripts tailored for DIY applications.

## Projects

### Water Usage Monitor
I created this project because I wanted a water flow sensor to help determine when and how much water is being used on this (100GPD) RO filter I got from amazon. It also assists with knowing around when to replace the filters for the system. Both wired and wireless should work but let me explain:

* The "UNOwatermonitoing" folder contains code for the (wired) [ArduinoUNO](https://www.amazon.com/gp/product/B008GRTSV6/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1), using [this hall effect sensor](https://www.amazon.com/gp/product/B07QS17S6Q/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)
* The "WiFiAdvancedWaterFlowMonitor" folder contains code for the (wireless) [Arduino UNO R4 Wifi]

**Key features of the project:**
Both scripts display water useage measurement in gallons, pretty accurately too! 😎
The wireless version of the code has a simple dashboard displaying water usage (see the example).
The wired version has a python script that prints output from the console but, this script still needs work in order to achieve similar functionality to the wireless version.

* **Live Demo:** [Water Usage Monitor App](http://wum.molovestoshare.com:8080) - Track water flow using a YF-S201 sensor and serve live data through a simple web interface.
* **Credits:**

  * [Water flow sensor project](https://forum.arduino.cc/t/water-flow-sensor-project/646119)
  * [Random Nerd Tutorials - ESP32 Web Server Arduino IDE](https://randomnerdtutorials.com/esp32-web-server-arduino-ide/)


### DWC Bucket Temperature Monitor

Real-time temperature monitoring and automatic alerts for Deep Water Culture (DWC) systems.

**Features:**

* Reads water temperature in a 5-gallon DWC bucket every minute
* Connects via Wi-Fi using Nano RP2040 Connect's NINA-W102 module
* Sends HTTP POST alerts through IFTTT Maker Webhooks when temperature drifts outside a configurable band (default **16–22 °C**)
* Logs readings and alert events over the Serial console

**Hardware Requirements:**

* Arduino Nano RP2040 Connect
* Waterproof DS18B20 temperature sensor + adapter board
* 4.7 KΩ pull-up resistor (if not included on adapter)
* Jumper wires or breadboard

**Software Dependencies (Library Manager):**

* WiFiNINA
* OneWire
* DallasTemperature

---

## Installation & Setup

1. **Clone the repository:**

   ```bash
   git clone https://github.com/your-username/arduino-projects.git
   ```
2. **Open the DWC monitor sketch:**

   * File: `DWC_Temp_Monitor.ino`
3. **Install libraries** via **Sketch → Include Library → Manage Libraries...**
4. **Configure your settings** in the sketch top section:

   ```cpp
   const char* SSID       = "YOUR_SSID";
   const char* PASS       = "YOUR_PASSWORD";
   const char* IFTTT_KEY  = "YOUR_IFTTT_KEY";
   const char* EVENT_NAME = "dwc_temp_alert";
   ```
5. **Wire the DS18B20 sensor:**

   * **VCC (red)** → 3.3 V
   * **GND (black)** → GND
   * **DATA (yellow)** → Pin D4 (with 4.7 KΩ pull-up to 3.3 V)
6. **Upload** to your Nano RP2040 Connect.
7. **Open Serial Monitor** at **115200 bauds** and watch temperature logs.

---

## Usage

* The sketch reads and prints the temperature in °C every 60 s.
* If the temperature falls below **16 °C** or exceeds **22 °C**, it triggers an IFTTT webhook to send an SMS or app notification.

## Contributing

Contributions are welcome! Please:

1. Fork this repo
2. Create a branch: `git checkout -b feature/your-feature`
3. Make changes and commit: `git commit -m "Add your feature"`
4. Push to the branch: `git push origin feature/your-feature`
5. Open a Pull Request

## License

This project is licensed under the [MIT License](LICENSE).
