// Written for the Nano RP2040 Connect
#include <WiFiNINA.h>
#include <WiFiClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ── USER CONFIG ───────────────────────────────────────────────────────────────
const char* ssid[]       = "SSID";
const char* pass[]       = "PASS";
const char* HOST       = "maker.ifttt.com";
const uint16_t PORT    = 80;
const char* ifttt_key[]  = "IFTTT_KEY";
const char* EVENT_NAME = "dwc_temp_alert";
// Temperature thresholds (°C)
const float TEMP_LOW   = 16.0;
const float TEMP_HIGH  = 22.0;
// Read interval (ms)
const unsigned long READ_INTERVAL = 60UL * 1000UL;
// ── END CONFIG ────────────────────────────────────────────────────────────────

// Data pin for DS18B20
#define ONE_WIRE_PIN 2
OneWire       oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);

// Wi‑Fi client
WiFiClient client;

// IFTTT host/port
const char* IFTTT_HOST = "maker.ifttt.com";
const uint16_t IFTTT_PORT = 80;

void setup() {
  Serial.begin(115200);
  while (!Serial) ;

  // Start sensor
  sensors.begin();

  // Check WiFiNINA module
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("! WiFiNINA module not found");
    while (true) delay(1000);
  }

  // Connect to Wi‑Fi
  Serial.print("Connecting to Wi‑Fi");
  int attempts = 0;
  while (WiFi.begin(SSID, PASS) != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
    if (++attempts > 60) {
      Serial.println("\n! Failed to connect");
      while (true) delay(1000);
    }
  }
  Serial.println("\nWi‑Fi connected, IP: " + WiFi.localIP().toString());
}

void sendAlert(float temp) {
  if (!client.connect(IFTTT_HOST, IFTTT_PORT)) {
    Serial.println("! Connection failed");
    return;
  }

  // Build URL and JSON body
  String url  = String("/trigger/") + EVENT_NAME + "/with/key/" + IFTTT_KEY;
  String body = String("{\"value1\":") + String(temp,1) + "}";

  // Send HTTP POST
  client.print("POST " + url + " HTTP/1.1\r\n");
  client.print("Host: " + String(IFTTT_HOST) + "\r\n");
  client.print("Content-Type: application/json\r\n");
  client.print("Content-Length: " + String(body.length()) + "\r\n");
  client.print("Connection: close\r\n\r\n");
  client.print(body);

  // Read & dump response (optional)
  while (client.connected()) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);
    }
  }
  client.stop();
}

void loop() {
  static unsigned long lastMillis = 0;
  unsigned long now = millis();
  if (now - lastMillis < READ_INTERVAL) return;
  lastMillis = now;

  // Read temperature
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  Serial.printf("Temp: %.1f °C\n", tempC);

  // If out of bounds, alert
  if (tempC < TEMP_LOW || tempC > TEMP_HIGH) {
    Serial.println("🌡️ Out of range, sending IFTTT alert");
    sendAlert(tempC);
  }
}