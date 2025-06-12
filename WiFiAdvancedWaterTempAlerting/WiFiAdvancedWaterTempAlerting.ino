/**********************************************************************
 * 🌊💧📶🌡️🤮🦉  DWC Water‑Temp Guardian for Arduino R4 WiFi            *
 * ------------------------------------------------------------------ *
 * • Samples water temperature every 10 s (DS18B20 on D2)
 * • LED matrix scrolls latest reading every 5 s
 * • Sends data to Telegraf monitoring
 * • Alerts via IFTTT if temp out of range for ≥60 s
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
#define MONITOR_HOST      "tgt_host"   // Replace with your Telegraf host
#define MONITOR_PORT      8125
#define IFTTT_EVENT_NAME  "water_temp_alert"
#define IFTTT_HOST        "maker.ifttt.com"
#define IFTTT_PORT        443

constexpr uint32_t SAMPLE_INTERVAL_MS = 10UL * 1000UL;     // 10 s
constexpr uint32_t LOG_INTERVAL_MS     = 60UL * 1000UL;   // 1 min (reserved)
constexpr uint32_t SCROLL_INTERVAL_MS = 5UL  * 1000UL;     // 5 s
constexpr uint32_t ALERT_COOLDOWN_MS  = 5UL * 60UL * 1000UL; // 5 m
constexpr uint32_t OUT_OF_RANGE_MS    = 60UL * 1000UL;     // 60 s

constexpr float TEMP_LOW_THRESHOLD  = 16; // <16°C = 🥶Reduced Growth/Plant Shock/Stress
constexpr float TEMP_HIGH_THRESHOLD = 23; // >23°C = 🦠Pathogen Risk

/* ─────── GLOBALS ─────── */
OneWire           oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensors(&oneWire);
ArduinoLEDMatrix  matrix;
WiFiClient        monitorClient;
WiFiSSLClient     sslClient;
HttpClient        httpClient(sslClient, IFTTT_HOST, IFTTT_PORT);
WiFiUDP udp;

// WiFi credentials
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
// telegraf host
char tgt_host[] = hostname_or_ip;

uint32_t lastSample = 0,
lastScroll = 0,
lastAlertTime = 0,
outOfRangeStart = 0,
lastErrorTime = 0;

float  lastTemp = NAN;
String lastMonitorError = "";
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
  
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  lastSample = lastScroll = millis();
}

/* ─────── MAIN LOOP ─────── */
void loop() {
  uint32_t now = millis();

  /* ── Temperature sample & monitoring ── */
  if (now - lastSample >= SAMPLE_INTERVAL_MS) {
    lastSample = now;
    sensors.requestTemperatures();
    lastTemp = sensors.getTempCByIndex(0);
    Serial.print("Temp: "); Serial.println(lastTemp,1);
    sendMonitorUpdate(lastTemp);

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
    matrix.beginDraw();
    matrix.clear();
    char buf[8]; dtostrf(lastTemp, 4, 1, buf);
    matrix.textFont(Font_5x7);
    matrix.textScrollSpeed(200);
    matrix.beginText(0,1,0xFFFFFFFF);
    matrix.println(buf);
    matrix.endText(SCROLL_LEFT);
  } else {
    matrix.endDraw();
  }
}
