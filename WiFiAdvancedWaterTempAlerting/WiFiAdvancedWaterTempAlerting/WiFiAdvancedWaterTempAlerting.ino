/*
  UNO R4 WiFi  · DS18B20  · IFTTT SMS  · scrolling temp on 12×8 LED matrix
  ------------------------------------------------------------------------
  • Sensor data pin →  D2  (with 4.7 kΩ pull-up to 5 V)
  • Temp alarms     →  <17 °C  or  >22 °C
  • One SMS every   →  15 min max
  • Matrix shows the live value as smooth, left-scrolling text
*/

#include <WiFiS3.h>
#include <WiFiSSLClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoHttpClient.h>

#include <ArduinoGraphics.h>     // needed for scrolling functions
#include <Arduino_LED_Matrix.h>
ArduinoLEDMatrix matrix;

#include "secrets.h"             // defines SECRET_SSID / SECRET_PASS / SECRET_IFTTT_KEY

/* ── HARDWARE ────────────────────────────────────────────────────────── */
constexpr uint8_t ONE_WIRE_BUS = 2;   // DS18B20 on pin D2
OneWire           oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

/* ── IFTTT  (HTTPS) ──────────────────────────────────────────────────── */
const char* EVENT_NAME = "dwc_temp_alert";
const char* HOST       = "maker.ifttt.com";
WiFiSSLClient ssl;
HttpClient    client(ssl, HOST, 443);

/* ── ALARM SETTINGS ──────────────────────────────────────────────────── */
constexpr float    TEMP_LOW_C  = 17.0;
constexpr float    TEMP_HIGH_C = 22.0;
constexpr uint32_t COOLDOWN_MS = 15UL * 60UL * 1000UL;

/* ── TIMERS ──────────────────────────────────────────────────────────── */
constexpr uint32_t SAMPLE_MS   = 30UL * 1000UL;     // sensor read period
uint32_t           lastSample  = 0;
uint32_t           lastAlert   = 0;

/* ── HELPERS ─────────────────────────────────────────────────────────── */
void sendIFTTT(float tempC)
{
  String path = String("/trigger/") + EVENT_NAME +
                "/json/with/key/" + SECRET_IFTTT_KEY;
  String payload = "{\"value1\":\"" + String(tempC, 1) + " °C\"}";

  client.beginRequest();
  client.post(path);
  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Content-Length", payload.length());
  client.endRequest();
  client.print(payload);

  Serial.print("IFTTT status = ");
  Serial.println(client.responseStatusCode());
  client.stop();
}

void connectWiFi()
{
  static bool announced = false;

  if (WiFi.status() == WL_CONNECTED) {
    if (!announced) {
      Serial.print("[Wi-Fi] connected, IP: ");
      Serial.println(WiFi.localIP());
      announced = true;
    }
    return;
  }

  announced = false;
  Serial.print("[Wi-Fi] connecting");
  WiFi.begin(SECRET_SSID, SECRET_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(250);
  }
  Serial.print("\n[Wi-Fi] connected, IP: ");
  Serial.println(WiFi.localIP());
  announced = true;
}

void showTempOnMatrix(float tempC)
{
  /* Build text like "23.4C " (space at end keeps gap between repeats) */
  char buf[8];
  dtostrf(tempC, 4, 1, buf);      // width 4, 1 decimal
  strcat(buf, "C ");

  matrix.beginDraw();
  matrix.textScrollSpeed(55);     // tweak 10-100 for faster/slower scroll :contentReference[oaicite:0]{index=0}
  matrix.textFont(Font_5x7);
  matrix.beginText(12, 0, 0xFFFFFF);   // start off-screen right
  matrix.print(buf);
  matrix.endText(SCROLL_LEFT);    // automatic left scroll :contentReference[oaicite:1]{index=1}
  matrix.endDraw();
}

/* ── ARDUINO SETUP ───────────────────────────────────────────────────── */
void setup()
{
  Serial.begin(9600);
  connectWiFi();

  sensors.begin();
  Serial.print("DS18B20 devices: ");
  Serial.println(sensors.getDeviceCount());

  matrix.begin();                 // initialise the 12×8 display
}

/* ── MAIN LOOP ───────────────────────────────────────────────────────── */
void loop()
{
  connectWiFi();                  // keep link alive

  uint32_t now = millis();

  /* -------- temperature sample every 30 s -------- */
  static float lastTemp = NAN;
  if (now - lastSample >= SAMPLE_MS) {
    lastSample = now;

    sensors.requestTemperatures();
    float tC = sensors.getTempCByIndex(0);

    if (tC == DEVICE_DISCONNECTED_C) {
      Serial.println("Sensor error ❌");
    } else {
      lastTemp = tC;

      Serial.print("🌡️  Temp: ");
      Serial.print(tC, 1);
      Serial.println(" °C");

      bool outOfRange = (tC < TEMP_LOW_C) || (tC > TEMP_HIGH_C);
      bool cooledDown = (now - lastAlert >= COOLDOWN_MS);

      if (outOfRange && cooledDown) {
        sendIFTTT(tC);
        lastAlert = now;
      }
    }
  }

  /* -------- always keep the matrix scrolling -------- */
  if (!isnan(lastTemp)) {
    showTempOnMatrix(lastTemp);
  }

  /* short delay keeps scrolling smooth without blocking Wi-Fi */
  delay(80);
}
