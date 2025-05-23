/**********************************************************************
 * 🌊💧📶🌡️🧐🦉  DWC Water‑Temp Guardian for Arduino R4 WiFi            *
 * ------------------------------------------------------------------ *
 * • Samples water temperature every 10 s (DS18B20 on D2)
 * • Logs one **minute‑averaged** value (°C × 10, int16) — 2880 slots ≈ 5.8 kB RAM
 * • Dark web page (auto‑refresh 5 s) shows current temp + 24/48/72 h low/high
 * • LED matrix scrolls latest reading every 5 s
 * • Alerts via IFTTT if temp < 17 °C or > 21 °C for ≥60 s, max once/5 min
 *********************************************************************/
#include <WiFiS3.h>
#include <WiFiSSLClient.h>
#include <ArduinoHttpClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>
#include <math.h>
#include "arduino_secrets.h"

/* ─────── CONSTANTS ─────── */
#define ONE_WIRE_BUS_PIN   2
#define IFTTT_EVENT_NAME   "water_temp_alert"
#define IFTTT_HOST         "maker.ifttt.com"
#define IFTTT_PORT         443
#define MONITOR_HOST       "192.168.50.233"
#define MONITOR_PORT       8125

constexpr float    TEMP_HIGH_THRESHOLD = 21.0;
constexpr float    TEMP_LOW_THRESHOLD  = 17.0;
constexpr uint32_t SAMPLE_INTERVAL_MS  = 10UL * 1000UL;   // 10 s
constexpr uint32_t LOG_INTERVAL_MS     = 60UL * 1000UL;   // 1 min
constexpr uint32_t OUT_OF_RANGE_MS     = 60UL * 1000UL;   // 60 s
constexpr uint32_t ALERT_COOLDOWN_MS   = 5UL * 60UL * 1000UL; // 5 m
constexpr uint32_t SCROLL_INTERVAL_MS  = 5UL * 1000UL;       // 5 s

constexpr uint16_t HISTORY_MINUTES = 48 * 60;              // 2880              // 4320

/* ─────── GLOBALS ─────── */
OneWire           oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensors(&oneWire);
ArduinoLEDMatrix  matrix;
WiFiSSLClient     sslClient;
HttpClient        httpClient(sslClient, IFTTT_HOST, IFTTT_PORT);
WiFiClient        monitorClient;
WiFiServer        server(80);

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

uint32_t lastSample = 0,
         lastLog    = 0,
         lastAlertTime = 0,
         outOfRangeStartMs = 0,
         lastErrorTime = 0,
         lastScroll = 0;

bool   inRange  = true;
float  lastTemp = 0.0;
String lastMonitorError = "";  // Track last monitor error

/* minute‑averaged history (store °C ×10 as int16_t to save RAM) */
int16_t  hist[HISTORY_MINUTES];
uint16_t histIdx   = 0;
uint16_t histCount = 0;
float    minuteSum = 0;
uint8_t  minuteCnt = 0;

/* ─────── helpers ─────── */
String cellStr(float v){ return isnan(v)? "—" : String(v,1)+"°"; }

void minMaxLast(uint16_t minutes,float &lo,float &hi){
  if(histCount==0){ lo = hi = NAN; return; }
  uint16_t span = minutes > histCount ? histCount : minutes;
  uint16_t start = (histIdx + HISTORY_MINUTES - span) % HISTORY_MINUTES;
  lo = 1000; hi = -1000;
  for(uint16_t i=0;i<span;i++){
    float v = hist[(start+i)%HISTORY_MINUTES] / 10.0; // back to °C
    if(v<lo) lo=v; if(v>hi) hi=v;
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

void sendMonitorUpdate(float t) {
  String payload = "{\"source\":\"dwc_r4wifi\",\"temperature\":{\"value\":" + String(t,1) + "}}";
  uint32_t now = millis();
  
  // Clear old errors after timeout
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
    
    // Wait for response
    while(monitorClient.available()) {
      monitorClient.read();
    }
    monitorClient.stop();
    lastMonitorError = ""; // Clear error on successful send
  } else {
    lastMonitorError = "Telegraf push failed"; // Set error message
    lastErrorTime = now; // Update error timestamp
  }
}

void serveHome(WiFiClient &c,float cur){
  float lo24,hi24,lo48,hi48,lo72,hi72;
  minMaxLast(24*60,lo24,hi24);
  minMaxLast(48*60,lo48,hi48);
  minMaxLast(72*60,lo72,hi72);

  String html =
    "<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<meta http-equiv='refresh' content='5'><title>DWC Temp</title>"
    "<style>html,body{height:100%;margin:0}body{display:flex;flex-direction:column;align-items:center;background:#121212;color:#e0e0e0;font-family:Arial,sans-serif;text-align:center}h1{margin:0;font-size:9vw}table{border-collapse:collapse;margin-top:1rem}th,td{border:1px solid #999;padding:6px 12px}th{background:#0d5c7d;color:#fff}</style></head><body>";
  html += "<h1>🌡️ "+String(cur,1)+" °C</h1>";
  html += "<div style='font-size:5vw;margin-top:0.3rem'>🌊 Current 💧 DWC&nbsp;Temp 🌡️</div>";
  html += "<table><tr><th colspan='4'>Temperature Over The Last</th></tr><tr><th></th><th>24 HRS</th><th>48 HRS</th><th>72 HRS</th></tr>";
  html += "<tr><td>Low:</td><td>"+cellStr(lo24)+"</td><td>"+cellStr(lo48)+"</td><td>"+cellStr(lo72)+"</td></tr>";
  html += "<tr><td>High:</td><td>"+cellStr(hi24)+"</td><td>"+cellStr(hi48)+"</td><td>"+cellStr(hi72)+"</td></tr></table>";
  if (lastMonitorError.length() > 0) {
    html += "<div style='color:#ff4444;margin-top:1rem'>⚠️ " + lastMonitorError + "</div>";
  }
  html += "</body></html>";
  c.print("HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nConnection: close\r\n\r\n");
  c.print(html);
}

/* ─────── SETUP ─────── */
void setup(){
  Serial.begin(9600);
  matrix.begin();
  sensors.begin();
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  server.begin();
  lastSample = lastLog = lastScroll = millis();
}

/* ─────── MAIN LOOP ─────── */
void loop(){
  if (WiFiClient client = server.available()){
    if(client){
      client.readStringUntil('\r');
      while(client.available()) client.read();
      serveHome(client, lastTemp);
      client.stop();
    }
  }

  uint32_t now = millis();

  // 10‑second sampling
  if(now - lastSample >= SAMPLE_INTERVAL_MS){
    lastSample = now;
    sensors.requestTemperatures();
    float t = sensors.getTempCByIndex(0);
    lastTemp = t;
    Serial.print("Temp: "); Serial.println(t,1);
    
    // Send temperature update to monitor
    sendMonitorUpdate(t);

    minuteSum += t; minuteCnt++;

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

  // minute‑average logging
  if(now - lastLog >= LOG_INTERVAL_MS){
    lastLog = now;
    if(minuteCnt == 0) minuteCnt = 1;
    float avg = minuteSum / minuteCnt;
    hist[histIdx] = int16_t(round(avg * 10));
    histIdx = (histIdx + 1) % HISTORY_MINUTES;
    if(histCount < HISTORY_MINUTES) histCount++;
    minuteSum = 0; minuteCnt = 0;
  }

  // LED scroll every 5 s
  if(now - lastScroll >= SCROLL_INTERVAL_MS){
    lastScroll = now;
    char buf[8]; dtostrf(lastTemp,4,1,buf);
    matrix.beginDraw();
    matrix.stroke(0xFFFFFFFF);
    matrix.textFont(Font_5x7);
    matrix.textScrollSpeed(200);
    matrix.beginText(0,1,0xFFFFFFFF);
    matrix.println(buf);
    matrix.endText(SCROLL_LEFT);
    matrix.endDraw();
  }
}
