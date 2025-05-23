/**********************************************************************
 *   DWC Water-Temp Guardian for Arduino R4 WiFi                       *
 *********************************************************************/
#include <WiFiS3.h>
#include <WiFiClient.h>
#include <ArduinoHttpClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "arduino_secrets.h"

#define ONE_WIRE_BUS_PIN    2
#define IFTTT_EVENT_NAME    "water_temp_alert"
#define IFTTT_HOST          "maker.ifttt.com"
#define TEMP_HIGH_THRESHOLD 21.0
#define TEMP_LOW_THRESHOLD  17.0
#define ALERT_COOLDOWN_MS   900000UL   // 15 minutes
#define SAMPLE_INTERVAL_MS 10000       // 10 seconds
#define MAX_HISTORY         144        // 24h @ 10min intervals
#define ENTRIES_PER_PAGE    24

float tempHistory[MAX_HISTORY];
String timeHistory[MAX_HISTORY];
uint16_t historyIndex = 0;
uint16_t historyCount = 0;
uint32_t lastLogTime = 0;

OneWire oneWire(ONE_WIRE_BUS_PIN);
DallasTemperature sensors(&oneWire);
ArduinoLEDMatrix matrix;
WiFiClient wifiClient;
HttpClient httpClient(wifiClient, IFTTT_HOST, 80);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -18000, 60000);
uint32_t lastSample = 0;
uint32_t lastAlertTime = 0;
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
WiFiServer server(80);

void initHistory() {
  for (int i = 0; i < MAX_HISTORY; i++) {
    tempHistory[i] = NAN;
    timeHistory[i] = "";
  }
  historyIndex = historyCount = 0;
}

void setup() {
  server.begin();
  initHistory();
  Serial.begin(9600);
  while (!Serial);
  matrix.loadSequence(LEDMATRIX_ANIMATION_STARTUP);
  matrix.begin();
  matrix.play(true);
  sensors.begin();
  timeClient.begin(); timeClient.update();
  Serial.print("📶 Connecting to "); Serial.print(ssid); Serial.print("...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print('.'); }
  Serial.println(); Serial.print("🛜 WiFi connected: "); Serial.println(WiFi.localIP());
  lastSample = millis() - SAMPLE_INTERVAL_MS;
}

void loop() {
  handleClient();
  uint32_t now = millis();
  float temp = sensors.getTempCByIndex(0);

  // Log every 10 minutes
  if (now - lastLogTime >= 600000UL) { logTemperature(temp); lastLogTime = now; }

  // Sample and alert
  if (now - lastSample >= SAMPLE_INTERVAL_MS) {
    lastSample = now;
    sensors.requestTemperatures();
    float tC = sensors.getTempCByIndex(0);
    if (tC <= -100.0) { Serial.println("DS18B20 read error"); return; }
    // Display
    Serial.print("Temperature: "); Serial.print(tC,1); Serial.println(" °C");
    char buf[7]; dtostrf(tC,4,1,buf);
    matrix.beginDraw(); matrix.stroke(0xFFFFFFFF); matrix.textFont(Font_5x7);
    matrix.textScrollSpeed(200); matrix.beginText(0,1,0xFFFFFFFF);
    matrix.println(buf); matrix.endText(SCROLL_LEFT); matrix.endDraw();
    // Threshold alert with cooldown
    if ((tC < TEMP_LOW_THRESHOLD || tC > TEMP_HIGH_THRESHOLD) && (now - lastAlertTime >= ALERT_COOLDOWN_MS)) {
      sendIFTTTAlert(tC);
      lastAlertTime = now;
    }
  }
}

void sendIFTTTAlert(float t) {
  String path = "/trigger/" + String(IFTTT_EVENT_NAME) + "/with/key/" + SECRET_IFTTT_KEY + "?value1=" + String(t,1);
  Serial.print("IFTTT GET "); Serial.println(path);
  httpClient.get(path);
  Serial.print("IFTTT: "); Serial.print(httpClient.responseStatusCode()); Serial.print(" / "); Serial.println(httpClient.responseBody());
}

void handleClient() {
  WiFiClient client = server.available(); if (!client) return;
  String req = client.readStringUntil('\r'); client.read();
  if (req.indexOf("GET /reset")>=0) initHistory();
  if (req.indexOf("GET /reset_heatmap")>=0) resetHeatmap();
  float currentTemp = sensors.getTempCByIndex(0); logToHeatmap(currentTemp);
  int page = 0;
  int entriesPerPage = ENTRIES_PER_PAGE;
  int totalEntries = historyCount;
  int totalPages = (totalEntries + entriesPerPage - 1) / entriesPerPage;
  int paramIdx = req.indexOf("page=");
  if (paramIdx >= 0) page = constrain(req.substring(paramIdx+5).toInt(), 0, totalPages-1);
  int startIdx = page * entriesPerPage;
  int endIdx = min(startIdx + entriesPerPage, totalEntries);
  String html = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n";
  html += "<html><head><meta http-equiv='refresh' content='15'>";
  html += "<style>body{background:#121212;color:#f0f0f0;font-family:sans-serif;margin:0;padding:1rem;}a{color:#90caf9;}ul{list-style:none;padding:0;}li{margin:4px 0;}table{border-collapse:collapse;width:100%;max-width:100%;overflow-x:auto;}th,td{border:1px solid #333;text-align:center;padding:4px;font-size:0.9rem;}@media(max-width:600px){body{padding:.5rem;}li{font-size:.8rem;}th,td{font-size:0.75rem;padding:2px;}}</style></head><body>";
  html += "<h2>DWC Water Temperature (Last 24h)</h2>";
  html += "<p>Current Temp: 🌡️ " + String(currentTemp,1) + " &deg;C</p><p><a href='/reset'>Reset History</a></p>";
  html += "<h3>Temperature History (Page " + String(page+1) + " of " + String(totalPages) + ")</h3><ul>";
  for (int i = startIdx; i < endIdx; i++) {
    int idx = (historyIndex + MAX_HISTORY - historyCount + i) % MAX_HISTORY;
    html += "<li>" + String(i+1) + ": 🌡️ " + String(tempHistory[idx],1) + " &deg;C @ " + timeHistory[idx] + "</li>";
  }
  html += "</ul><p>" + (page>0?"<a href='/?page="+String(page-1)+"'>&lt;Prev</a> ":"") + (page<totalPages-1?"<a href='/?page="+String(page+1)+"'>Next&gt;</a>"
  :"") + "</p>";
  html += renderHeatmapHTML();
  html += "</body></html>";
  client.print(html); client.stop();
}

void logTemperature(float temp) {
  timeClient.update();
  tempHistory[historyIndex] = temp;
  timeHistory[historyIndex] = timeClient.getFormattedTime();
  historyIndex = (historyIndex + 1) % MAX_HISTORY;
  if (historyCount < MAX_HISTORY) historyCount++;
}

float tempByHour[7][24]; bool heatmapFilled[7][24];
void initHeatmap() { for (int d=0; d<7; d++) for (int h=0; h<24; h++) { tempByHour[d][h]=NAN; heatmapFilled[d][h]=false; }}
void resetHeatmap() { initHeatmap(); }
void logToHeatmap(float temp) {
  float prev = historyCount ? tempHistory[(historyIndex + MAX_HISTORY -1) % MAX_HISTORY] : temp;
  float avg = (prev + temp) / 2.0;
  timeClient.update(); unsigned long epoch = timeClient.getEpochTime();
  int hr = (epoch % 86400UL) / 3600;
  int dy = ((epoch / 86400UL) + 4) % 7;
  tempByHour[dy][hr] = avg;
  heatmapFilled[dy][hr] = true;
}
String renderHeatmapHTML() {
  const char* days[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  String html = "<h3>Heatmap: Hour × Day</h3><table border='1'><tr><th>Hour</th>";
  for(int d=0; d<7; d++) html += "<th>" + String(days[d]) + "</th>";
  html += "</tr>";
  for(int h=0; h<24; h++) {
    html += "<tr><td>" + String(h) + ":00</td>";
    for(int d=0; d<7; d++) {
      if(heatmapFilled[d][h]) {
        float t = tempByHour[d][h];
        if(t < TEMP_LOW_THRESHOLD) html += "<td>🟦</td>";
        else if(t <= TEMP_HIGH_THRESHOLD) html += "<td>🟩</td>";
        else html += "<td>🟥</td>";
      } else html += "<td>⬜</td>";
    }
    html += "</tr>";
  }
  html += "</table><p><a href='/reset_heatmap'>Reset Heatmap</a></p>";
  return html;
}
