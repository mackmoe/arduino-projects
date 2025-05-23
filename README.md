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

## 🚀 Getting Started

1. 🔌 **Wire up**  
   - DS18B20 data → D2  
   - 12×8 LED matrix to SPI pins  

2. 📝 **Configure**  
   - Copy your Wi-Fi & IFTTT key into `arduino_secrets.h`  
   - (Optional) Adjust `TEMP_LOW_THRESHOLD`, `TEMP_HIGH_THRESHOLD`, or `ALERT_COOLDOWN_MS`  

3. 📦 **Upload**  
   - Select **Arduino R4 WiFi** board  
   - Compile & upload the sketch  

4. 🌐 **View Dashboard**  
   - Point your browser at `http://<arduino-ip>/`  
   - **Current Temp** auto-refreshes (60 s)  
   - Browse history pages with **Prev/Next**  

5. 🔔 **Alerts**  
   - Sends IFTTT SMS when temp goes out of range  
   - Respects 15 min cool-down (`ALERT_COOLDOWN_MS`)  

6. 🔄 **Reset & Heatmap**  
   - Click **Reset History** to clear logs  
   - Scroll down to see a 24 h heatmap of readings  
---

## Usage

* The sketch reads and prints the temperature in °C every 60 s.
* If the temperature falls below **16 °C** or exceeds **21 °C**, it triggers an IFTTT webhook to send an SMS notification.

## Contributing

Contributions are welcome! Please:

1. Fork this repo
2. Create a branch: `git checkout -b feature/your-feature`
3. Make changes and commit: `git commit -m "Add your feature"`
4. Push to the branch: `git push origin feature/your-feature`
5. Open a Pull Request

## License

This project is licensed under the [MIT License](LICENSE).
