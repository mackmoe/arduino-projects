/**********************************************************************
 * 🌊💧🌡️🧐  DWC Water‑Temp Guardian for Arduino R4 WiFi                *
 *********************************************************************/
// Core WiFi library for R4 WiFi
#include <WiFiS3.h>
#include <WiFiClient.h>
#include <ArduinoHttpClient.h>

// 1‑wire bus support & DS18B20 driver
#include <OneWire.h>
#include <DallasTemperature.h>

// 12×8 LED matrix stuff
#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>

#include "arduino_secrets.h"

int status = WL_IDLE_STATUS;

// ── User‑configurable pins & thresholds ─────────────────────────────
#define ONE_WIRE_BUS_PIN    2      // DS18B20 data connected here
#define IFTTT_EVENT_NAME    "water_temp_alert"
#define IFTTT_HOST          "maker.ifttt.com"

#define TEMP_HIGH_THRESHOLD 21.0   // °C upper bound
#define TEMP_LOW_THRESHOLD  17.0   // °C lower bound
#define SAMPLE_INTERVAL_MS 10000   // how often to read (10 s)
#define REPORT_INTERVAL_MS 3600000UL

#define HOURLY_EVENT_NAME     "water_temp_hourly"


// ── Global objects & state ─────────────────────────────────────────
OneWire        oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensors(&oneWire);
ArduinoLEDMatrix matrix;

WiFiClient     wifiClient;
HttpClient     httpClient(wifiClient, IFTTT_HOST, 80);

bool   alertSent = false;
uint32_t lastSample = 0;
uint32_t lastReport = 0;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

// ── Setup: init serial, display, sensor, WiFi ───────────────────────
void setup() {
  Serial.begin(9600);
  while (!Serial); // wait for Serial (if needed)

  // LED‑matrix startup animation
  matrix.loadSequence(LEDMATRIX_ANIMATION_STARTUP);
  matrix.begin();
  matrix.play(true);

  // Start DS18B20 sensor
  sensors.begin();

  // Connect to WiFi
  Serial.print("📶 Connecting to “");
  Serial.print(ssid);
  Serial.print("” …");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();
  Serial.print("🛜 WiFi is Connected to: ");
  Serial.println(WiFi.localIP());

  lastSample = millis() - SAMPLE_INTERVAL_MS;
}

// ── Main loop: sample temp, display, check thresholds ───────────────
void loop() {
  uint32_t now = millis();

  // — only when it’s time to sample do we read, display, and threshold —
  if (now - lastSample >= SAMPLE_INTERVAL_MS) {
    lastSample = now;

    // 1) Read the sensor
    sensors.requestTemperatures();
    float tC = sensors.getTempCByIndex(0);

    // 2) Guard against read‑errors
    if (tC <= -100.0) {
      Serial.println("⚠️ DS18B20 read error");
      return; // skip display & alerts this cycle
    }

    // 3) Print + display
    Serial.print("🌡️ Temperature: ");
    Serial.print(tC, 1);
    Serial.println(" °C");

    // — scrolling text on the 12×8 matrix —
    char buf[7];
    dtostrf(tC, 4, 1, buf);

    matrix.beginDraw();
      matrix.stroke(0xFFFFFFFF);
      matrix.textFont(Font_5x7);
      matrix.textScrollSpeed(200);           // ← you need this!
      matrix.beginText(0, 1, 0xFFFFFFFF);
      matrix.println(buf);
      matrix.endText(SCROLL_LEFT);
    matrix.endDraw();

    // 4) Threshold alert via IFTTT
    if ((tC < TEMP_LOW_THRESHOLD || tC > TEMP_HIGH_THRESHOLD) && !alertSent) {
      sendIFTTTAlert(tC);
      alertSent = true;
    }
    else if (tC >= TEMP_LOW_THRESHOLD && tC <= TEMP_HIGH_THRESHOLD) {
      alertSent = false;
    }
  }

  // — hourly (or test‑interval) report with a fresh read —
  if (now - lastReport >= REPORT_INTERVAL_MS) {
    lastReport = now;
    sensors.requestTemperatures();
    float tC_report = sensors.getTempCByIndex(0);
    sendHourlyReport(tC_report);
  }
}

// ── Helper: fire IFTTT webhook with temperature as value1 ───────────
void sendIFTTTAlert(float t) {
  String path = String("/trigger/") + IFTTT_EVENT_NAME +
                "/with/key/" + SECRET_IFTTT_KEY +
                "?value1=" + String(t,1);

  Serial.print("→ IFTTT GET ");
  Serial.println(path);

  httpClient.get(path);
  int code = httpClient.responseStatusCode();
  String body = httpClient.responseBody();
  Serial.print("🦉IFTTT responded: ");
  Serial.print(code);
  Serial.print(" / ");
  Serial.println(body);
}

void sendHourlyReport(float t) {
  String path = String("/trigger/") + HOURLY_EVENT_NAME
                + "/with/key/" + SECRET_IFTTT_KEY
                + "?value1=" + String(t,1);
  Serial.print("→ hourly GET ");
  Serial.println(path);
  httpClient.get(path);
  Serial.print("✉️ Hourly IFTTT: ");
  Serial.print(httpClient.responseStatusCode());
  Serial.print(" / ");
  Serial.println(httpClient.responseBody());
}
