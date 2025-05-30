/**********************************************************************
 * 🌊💧📶🌡️🤮🦉  DWC Water‑Temp Guardian for Arduino R4 WiFi            *
 * ------------------------------------------------------------------ *
 * • Samples water temperature every 10 s (DS18B20 on D2)
 * • LED matrix scrolls latest reading every 5 s
 * • Sends data to Telegraf monitoring
 * • Alerts via IFTTT if temp out of range for ≥60 s
 * • AUTOMATES CHILLER via relay output on pin 8
 * • Matrix shows status: spinning snowflake when chiller is ON + temp
 *********************************************************************/
#include <WiFiS3.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>
#include <math.h>
#include "arduino_secrets.h"
#include <WiFiSSLClient.h>
#include <ArduinoHttpClient.h>

/* ─────── CONSTANTS ─────── */
#define ONE_WIRE_BUS_PIN   2
#define RELAY_PIN          8
#define MONITOR_HOST       "INFLUXDB"
#define MONITOR_PORT       8125
#define IFTTT_EVENT_NAME   "water_temp_alert"
#define IFTTT_HOST         "maker.ifttt.com"
#define IFTTT_PORT         443

constexpr uint32_t SAMPLE_INTERVAL_MS  = 10UL * 1000UL;
constexpr uint32_t LOG_INTERVAL_MS     = 60UL * 1000UL;
constexpr uint32_t SCROLL_INTERVAL_MS  = 5UL * 1000UL;
constexpr uint32_t ALERT_COOLDOWN_MS   = 5UL * 60UL * 1000UL;
constexpr uint32_t OUT_OF_RANGE_MS     = 60UL * 1000UL;

constexpr float    TEMP_HIGH_THRESHOLD = 21.0;
constexpr float    TEMP_LOW_THRESHOLD  = 17.0;

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
         lastLog    = 0,
         lastErrorTime = 0,
         lastScroll = 0,
         lastAlertTime = 0,
         outOfRangeStartMs = 0;

float  lastTemp = 0.0;
String lastMonitorError = "";
bool   inRange = true;
bool   chillerOn = false;

/* ─────── helpers ─────── */
String cellStr(float v){ return isnan(v)? "—" : String(v,1)+"°"; }

void sendMonitorUpdate(float t) {
  String payload = "{\"source\":\"dwc_r4wifi\",\"temperature\":{\"value\":" + String(t,1) + "}}";
  uint32_t now = millis();

  if (lastMonitorError.length() > 0 && (now - lastErrorTime >= ALERT_COOLDOWN_MS)) {
    lastMonitorError = "";
  }

  if (monitorClient.connect(MONITOR_HOST, MONITOR_PORT)) {
    monitorClient.println("POST /dwc-temp-monitor HTTP/1.1");
    monitorClient.println("Host: " + String(MONITOR_HOST));
    monitorClient.println("Content-Type: application/json");
    monitorClient.println("Content-Length: " + String(payload.length()));
    monitorClient.println();
    monitorClient.println(payload);
    while(monitorClient.available()) { monitorClient.read(); }
    monitorClient.stop();
    lastMonitorError = "";
  } else {
    lastMonitorError = "Telegraf push failed";
    lastErrorTime = now;
  }
}

void sendIFTTTAlert(float t){
  String path = "/trigger/"+String(IFTTT_EVENT_NAME)+"/with/key/"+SECRET_IFTTT_KEY;
  String body = "{\"value1\":\""+String(t,1)+"\"}";
  httpClient.beginRequest();
  httpClient.post(path);
  httpClient.sendHeader("Content-Type","application/json");
  httpClient.sendHeader("Content-Length", body.length());
  httpClient.beginBody(); httpClient.print(body); httpClient.endRequest();
  while(httpClient.available()) httpClient.read(); httpClient.stop();
}

/* ─────── SETUP ─────── */
void setup(){
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  matrix.begin();
  sensors.begin();
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  lastSample = lastLog = lastScroll = millis();
}

// Static icons (after globals)
const uint8_t checkmarkIcon[8] = {
  B00000000,
  B00000010,
  B00000100,
  B00001000,
  B01010000,
  B00100000,
  B00000000,
  B00000000
};

const uint8_t noSmokingIcon[8] = {
  B10000001,
  B01000010,
  B00100100,
  B00011000,
  B00011000,
  B00100100,
  B01000010,
  B10000001
};

/* ─────── MAIN LOOP ─────── */
void loop(){
  uint32_t now = millis();

  if(now - lastSample >= SAMPLE_INTERVAL_MS){
    lastSample = now;
    sensors.requestTemperatures();
    float t = sensors.getTempCByIndex(0);
    lastTemp = t;
    Serial.print("Temp: "); Serial.println(t,1);

    sendMonitorUpdate(t);

    if(t > TEMP_HIGH_THRESHOLD){
      digitalWrite(RELAY_PIN, HIGH);
      chillerOn = true;
    } else if(t < TEMP_LOW_THRESHOLD){
      digitalWrite(RELAY_PIN, LOW);
      chillerOn = false;
    }

    bool out = (t < TEMP_LOW_THRESHOLD || t > TEMP_HIGH_THRESHOLD);
    if(out){
      if(inRange){ inRange = false; outOfRangeStartMs = now; }
      else if((now - outOfRangeStartMs >= OUT_OF_RANGE_MS) && (now - lastAlertTime >= ALERT_COOLDOWN_MS)){
        sendIFTTTAlert(t);
        lastAlertTime = now;
        outOfRangeStartMs = now;
      }
    } else {
      inRange = true;
    }
  }

  if(now - lastScroll >= SCROLL_INTERVAL_MS){
    lastScroll = now;
    char buf[8]; dtostrf(lastTemp,4,1,buf);
    matrix.beginDraw();
    matrix.clear();
    matrix.stroke(0xFFFFFFFF);
    
    // Draw static icon first
    const uint8_t* icon = chillerOn ? checkmarkIcon : noSmokingIcon;
    for(int y = 0; y < 8; y++) {
      uint8_t row = icon[y];
      for(int x = 0; x < 8; x++) {
        if(row & (1 << (7-x))) {
          matrix.point(x, y);
        }
      }
    }
    
    // Then draw scrolling text
    matrix.textFont(Font_5x7);
    matrix.textScrollSpeed(230);
    matrix.beginText(8, 1, 0xFFFFFFFF);
    matrix.println(buf);
    matrix.endText(SCROLL_LEFT);
    
    matrix.endDraw();
  }
}
