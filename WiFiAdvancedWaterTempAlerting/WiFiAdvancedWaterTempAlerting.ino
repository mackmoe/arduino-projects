/**********************************************************************
 * 🌊💧📶🌡️🤮🦉  DWC Water‑Temp Guardian for Arduino R4 WiFi            *
 * ------------------------------------------------------------------ *
 * • Samples water temperature every 10 s (DS18B20 on D2)
 * • LED matrix scrolls latest reading every 5 s
 * • Sends data to Telegraf monitoring
 * • Alerts via IFTTT if temp out of range for ≥60 s
 * • Drives TEC1‑12706 via IRLZ44N MOSFET on D8 (target band 17–20 °C)
 *********************************************************************/
#include <math.h>
#include <WiFiS3.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>
#include <WiFiSSLClient.h>
#include <ArduinoHttpClient.h>
#include "arduino_secrets.h"

/* ─────── CONSTANTS ─────── */
#define ONE_WIRE_BUS_PIN  2           // DS18B20 data gate
#define PELTIER_PIN       8           // Gate of IRLZ44N MOSFET
#define MONITOR_HOST      "hostname-or-ip"   // Replace with your Telegraf host
#define MONITOR_PORT      8125
#define IFTTT_EVENT_NAME  "water_temp_alert"
#define IFTTT_HOST        "maker.ifttt.com"
#define IFTTT_PORT        443

constexpr uint32_t SAMPLE_INTERVAL_MS = 10UL * 1000UL;     // 10 s
constexpr uint32_t LOG_INTERVAL_MS     = 60UL * 1000UL;   // 1 min (reserved)
constexpr uint32_t SCROLL_INTERVAL_MS = 5UL  * 1000UL;     // 5 s
constexpr uint32_t ALERT_COOLDOWN_MS  = 15UL * 60UL * 1000UL; // 15 m
constexpr uint32_t OUT_OF_RANGE_MS    = 60UL * 1000UL;     // 60 s

constexpr float TEMP_HIGH_THRESHOLD = 20.0;   // °C  → turn ON above
constexpr float TEMP_LOW_THRESHOLD  = 17.0;   // °C  → turn OFF below

/* ─────── GLOBALS ─────── */
OneWire           oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensors(&oneWire);
ArduinoLEDMatrix  matrix;
WiFiClient        monitorClient;
WiFiSSLClient     sslClient;
HttpClient        httpClient(sslClient, IFTTT_HOST, IFTTT_PORT);

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

uint32_t lastSample = 0,
         lastScroll = 0,
         lastAlertTime = 0,
         outOfRangeStart = 0,
         lastErrorTime = 0;

float  lastTemp = NAN;
String lastMonitorError = "";
bool   peltierOn = false;
bool   inRange   = true;

/* ─────── helpers ─────── */
void sendMonitorUpdate(float t) {
  // Original, Telegraf‑compatible payload build (do not modify!)
  String payload = "{\"source\":\"dwc_r4wifi\",\"temperature\":{\"value\":" + String(t,1) + "}}";
  uint32_t now = millis();
  // Clear old errors after timeout
  if (lastMonitorError.length() > 0 && (now - lastErrorTime >= ALERT_COOLDOWN_MS)) {
    lastMonitorError = "";
  }
  // Send to Telegraf
  if (monitorClient.connect(MONITOR_HOST, MONITOR_PORT)) {
    monitorClient.println("POST /dwc-temp-monitor HTTP/1.1");
    monitorClient.println("Host: " + String(MONITOR_HOST));
    monitorClient.println("Content-Type: application/json");
    monitorClient.println("Content-Length: " + String(payload.length()));
    monitorClient.println();
    monitorClient.println(payload);
    // Wait for response
    while (monitorClient.available()) monitorClient.read();
    monitorClient.stop();
    lastMonitorError = "";
  } else {
    lastMonitorError = "Telegraf push failed";
    lastErrorTime = now;
  }
}

void sendIFTTTAlert(float t) {
  String path = "/trigger/" + String(IFTTT_EVENT_NAME) + "/with/key/" + String(SECRET_IFTTT_KEY);
  String body = "{\"value1\":\"" + String(t,1) + "\"}";
  httpClient.beginRequest();
  httpClient.post(path);
  httpClient.sendHeader("Content-Type","application/json");
  httpClient.sendHeader("Content-Length", body.length());
  httpClient.beginBody(); httpClient.print(body); httpClient.endRequest();
  while (httpClient.available()) httpClient.read();
  httpClient.stop();
}

/* ─────── SETUP ─────── */
void setup() {
  Serial.begin(9600);
  matrix.begin();
  sensors.begin();
  pinMode(PELTIER_PIN, OUTPUT);  // MOSFET gate
  digitalWrite(PELTIER_PIN, LOW); // Peltier OFF at boot

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  lastSample = lastScroll = millis();
}

/* ─────── MAIN LOOP ─────── */
void loop() {
  uint32_t now = millis();

  /* ── Temperature sample & Peltier control ── */
  if (now - lastSample >= SAMPLE_INTERVAL_MS) {
    lastSample = now;
    sensors.requestTemperatures();
    lastTemp = sensors.getTempCByIndex(0);
    Serial.print("Temp: "); Serial.println(lastTemp,1);
    sendMonitorUpdate(lastTemp);

    /* ── Two‑level control ── */
    if (peltierOn) {
      if (lastTemp < TEMP_LOW_THRESHOLD) {
        digitalWrite(PELTIER_PIN, LOW);
        peltierOn = false;
      }
    } else {
      if (lastTemp > TEMP_HIGH_THRESHOLD) {
        digitalWrite(PELTIER_PIN, HIGH);
        peltierOn = true;
      }
    }

    /* ── Alert logic ── */
    bool out = (lastTemp < TEMP_LOW_THRESHOLD || lastTemp > TEMP_HIGH_THRESHOLD);
    if (out) {
      if (inRange) { inRange = false; outOfRangeStart = now; }
      else if ((now - outOfRangeStart >= OUT_OF_RANGE_MS) && (now - lastAlertTime >= ALERT_COOLDOWN_MS)) {
        sendIFTTTAlert(lastTemp);
        lastAlertTime = now;
        outOfRangeStart = now;
      }
    } else {
      inRange = true;
    }
  }

  /* ── LED control ── */
  if (now - lastScroll >= SCROLL_INTERVAL_MS) {
    lastScroll = now;
    static bool showTemp = true;
    
    matrix.beginDraw();
    matrix.clear();
    
    if (showTemp) {
      char buf[8]; dtostrf(lastTemp, 4, 1, buf);
      matrix.textFont(Font_5x7);
      matrix.textScrollSpeed(200);
      matrix.beginText(0,1,0xFFFFFFFF);
      matrix.println(buf);
      matrix.endText(SCROLL_LEFT);
    } else {
      if (peltierOn) {
        matrix.loadFrame(LEDMATRIX_EMOJI_SAD);
      } else {
        matrix.loadFrame(LEDMATRIX_EMOJI_HAPPY);
      }
    }
    
    matrix.endDraw();
    showTemp = !showTemp;  // Toggle between temperature and emoji
  }
}
