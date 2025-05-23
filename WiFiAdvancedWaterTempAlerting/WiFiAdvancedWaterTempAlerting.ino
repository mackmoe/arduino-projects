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



#define MAX_HISTORY 144
float tempHistory[MAX_HISTORY];
uint16_t historyIndex = 0;
uint16_t historyCount = 0;
uint32_t lastLogTime = 0;

void initHistory() {
  for (int i = 0; i < MAX_HISTORY; i++) {
    tempHistory[i] = NAN;
  }
  historyIndex = 0;
  historyCount = 0;
}

void logTemperature(float temp) {
  if (historyCount < MAX_HISTORY) historyCount++;
  tempHistory[historyIndex] = temp;
  historyIndex = (historyIndex + 1) % MAX_HISTORY;
}
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

WiFiServer server(80);  // HTTP server on port 80
char pass[] = SECRET_PASS;

// ── Setup: init serial, display, sensor, WiFi ───────────────────────
void setup() {
  server.begin();  // Start the HTTP server
  initHistory();
  initTime();
  initHeatmap();
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
  handleClient();

  float temp = sensors.getTempCByIndex(0);
  if (millis() - lastLogTime >= 300000UL) {
    logTemperature(temp);
    lastLogTime = millis();
  }
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


// ── HTTP client handler ─────────────────────────────────────────────
void handleClient() {
  WiFiClient client = server.available();
  if (client) {
    String req = client.readStringUntil('\r');
    client.flush();

    if (req.indexOf("GET /reset") >= 0) initHistory();
    if (req.indexOf("GET /reset_heatmap") >= 0) resetHeatmap();

    float currentTemp = sensors.getTempCByIndex(0);
    logToHeatmap(currentTemp);

    String html = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n";
    html += "<html><head>";
html += "<meta http-equiv='refresh' content='60'>";
html += "<style>";
html += "body { background-color: #121212; color: #f0f0f0; font-family: sans-serif; margin: 0; padding: 1rem; }";
html += "a { color: #90caf9; }";
html += "table { border-collapse: collapse; width: 100%; max-width: 100%; overflow-x: auto; display: block; }";
html += "th, td { border: 1px solid #333; text-align: center; padding: 4px; font-size: 0.9rem; }";
html += "@media (max-width: 600px) {";
html += "th, td { font-size: 0.75rem; padding: 2px; }";
html += "}";
html += "</style>";
html += "</head><body>";
    html += "<h2>DWC Water Temperature</h2>";
    html += "<p>Current Temp: " + String(currentTemp, 1) + " &deg;C</p>";
    html += "<p><a href='/reset'>Reset History</a></p>";
    html += "<h3>Temp History</h3><ul>";
    for (uint16_t i = 0; i < historyCount; i++) {
      uint16_t idx = (historyIndex + MAX_HISTORY - historyCount + i) % MAX_HISTORY;
      html += "<li>" + String(i + 1) + ": " + String(tempHistory[idx], 1) + " &deg;C</li>";
    }
    html += "</ul>";

    html += renderHeatmapHTML();
    html += "</body></html>";

    client.print(html);
    client.stop();
  }
}



// ── NTP Client Setup ────────────────────────────────────────────────
#include <WiFiUdp.h>
#include <NTPClient.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -18000, 60000);  // UTC-5 for CDT  // UTC time

// ── Heatmap Buffer ──────────────────────────────────────────────────
float tempByHour[7][24];  // day (0=Sun) x hour
bool heatmapFilled[7][24];

// ── Initialize NTP and heatmap ──────────────────────────────────────
void initTime() {
  timeClient.begin();
  timeClient.update();
}

void initHeatmap() {
  for (int d = 0; d < 7; d++) {
    for (int h = 0; h < 24; h++) {
      tempByHour[d][h] = NAN;
      heatmapFilled[d][h] = false;
    }
  }
}

// ── Reset heatmap ───────────────────────────────────────────────────
void resetHeatmap() {
  initHeatmap();
}

// ── Log temperature to NTP time slot ────────────────────────────────
void logToHeatmap(float temp) {
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  int h = (epochTime  % 86400UL) / 3600;          // hour of day
  int d = ((epochTime / 86400UL) + 4) % 7;        // day of week, 0 = Sun
  tempByHour[d][h] = temp;
  heatmapFilled[d][h] = true;
}

// ── Render heatmap HTML ─────────────────────────────────────────────
String renderHeatmapHTML() {
  const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  String html = "<h3>Heatmap: Hour × Day</h3><table border='1'><tr><th>Hour</th>";
  for (int d = 0; d < 7; d++) html += "<th>" + String(days[d]) + "</th>";
  html += "</tr>";
  for (int h = 0; h < 24; h++) {
    html += "<tr><td>" + String(h) + ":00</td>";
    for (int d = 0; d < 7; d++) {
      if (heatmapFilled[d][h]) {
        float t = tempByHour[d][h];
        if (t < 17.0) html += "<td>🟦</td>";
        else if (t <= 21.0) html += "<td>🟩</td>";
        else html += "<td>🟥</td>";
      } else {
        html += "<td>⬜</td>";
      }
    }
    html += "</tr>";
  }
  html += "</table><p><a href='/reset_heatmap'>Reset Heatmap</a></p>";
  return html;
}
