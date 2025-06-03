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
Here’s the complete README in raw Markdown you can copy-paste directly:

```markdown
# DWC Water-Temp Guardian for Arduino R4 WiFi

A self-contained temperature-control, monitoring, and alerting solution for Deep-Water Culture (DWC) hydroponic reservoirs.

* **Hardware**: Arduino UNO R4 WiFi, DS18B20 probe, TEC1-12706 Peltier module, IRLZ44N MOSFET (low-side), 12 V / 6 A PSU, on-board 12 × 8 LED matrix  
* **Features**
  * Samples water temperature every **10 s**
  * Maintains water between **17 °C – 20 °C** (MOSFET on D8)
  * Streams JSON metrics to **Telegraf** (HTTP listener on port 8125)
  * Sends **IFTTT** alerts if temp stays out-of-band ≥ 60 s (15-min cooldown)
  * Scrolls live temperature on the LED matrix every 5 s

---

## 1  Bill of Materials

| Qty | Part | Notes |
| --- | ---- | ----- |
| 1 | Arduino UNO R4 WiFi | ESP32-S3 coprocessor, LED matrix |
| 1 | DS18B20 waterproof sensor | + 4.7 kΩ pull-up |
| 1 | TEC1-12706 Peltier + heatsink/fan | good hot-side cooling **required** |
| 1 | IRLZ44N logic-level MOSFET | TO-220 package |
| 1 | 12 V ≥ 6 A DC supply | dedicated for Peltier |
| — | Wiring, 150 Ω gate resistor, 10 kΩ pulldown, Schottky diode | misc |

> ⚠️ The Arduino **only** drives the MOSFET gate. The TEC’s current flows directly from the 12 V supply.

---

## 2  Wiring

```

12 V(+) ───────► TEC +
TEC – ───► MOSFET D
MOSFET S ──────► GND  ◄── Arduino GND
Arduino D8 ─150 Ω─► MOSFET G
│
└─10 kΩ─► GND

````

Common ground is mandatory.  
Add a Schottky diode (e.g., 1N5819) across the TEC for surge protection.

---

## 3  Software Setup

### 3.1  Secrets

Create **`arduino_secrets.h`**:

```cpp
#define SECRET_SSID      "your-ssid"
#define SECRET_PASS      "your-wifi-password"
#define SECRET_IFTTT_KEY "xxxxxxxxxxxxxxxxxxxxxx"
````

### 3.2  Libraries

Install via Library Manager:

* WiFiS3
* OneWire
* DallasTemperature
* ArduinoHttpClient

### 3.3  Build & Flash

1. Open `dwc_water_temp_guardian_hysteresis.ino` in the Arduino IDE
2. Select **Arduino UNO R4 WiFi** board
3. *Verify* → *Upload*

---

## 4  Telegraf Quick Config

```toml
[[inputs.http_listener_v2]]
  service_address = ":8125"
  data_format     = "json_v2"
```

*Grafana gauge query (last 30 s):*

```flux
from(bucket: "dwc_temp_monitoring")
  |> range(start: -30s)
  |> filter(fn: (r) =>
       r._measurement == "http_listener_v2" and
       r._field == "temperature")
```

---

## 5  Control Logic

| Condition    | Action      | State           |
| ------------ | ----------- | --------------- |
| Temp > 20 °C | MOSFET HIGH | **Cooling ON**  |
| Temp < 17 °C | MOSFET LOW  | **Cooling OFF** |

LED matrix scrolls the latest reading; serial console logs raw values.

---

## 6  Troubleshooting

* **No Wi-Fi** → verify `SECRET_SSID/PASS`, 2.4 GHz only
* **No metrics** → check Telegraf listener `http://<host>:8125/dwc-temp-monitor`
* **Peltier always ON/OFF** → ensure hot-side heatsink fan is powered and DS18B20 probe is submerged

---

## 7  License

MIT © 2025 Mo

```
```
