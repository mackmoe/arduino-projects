/*
  RO water‑flow monitor – UNO R4 WiFi
  ------------------------------------------------
  * Interrupt‑driven pulse counting (Hall‑effect flow sensor)
  * Calculates L/min and cumulative volume
  * Publishes JSON to Telegraf over HTTP
  * Exposes /reset endpoint on port 8080 to zero counters
*/

#include <WiFiS3.h>
#include <ArduinoJson.h>
#include <Arduino_LED_Matrix.h>
#include "arduino_secrets.h"

/* ───── Wi‑Fi & Telegraf ───── */
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
const IPAddress TELEGRAF_HOST(xxx, xxx, xx, x);
constexpr uint16_t TELEGRAF_PORT = 8126;
constexpr char     URL_PATH[]    = "/ro-water-monitor";

/* ───── Flow sensor ───── */
constexpr byte  FLOW_PIN = 2;
constexpr float PULSES_PER_LITER = 3690.0;   // calibrate for your sensor 3830=TooHigh
constexpr uint32_t MEAS_MS = 1000;           // publish cadence (ms)

/* ───── Web control / telemetry ───── */
WiFiServer webServer(8080);

/* ───── LED‑matrix ───── */
ArduinoLEDMatrix matrix;
constexpr uint32_t ANIM_FRAME_INTERVAL = 3000; // ms per frame
uint32_t lastFrameAdvance = 0;

/* ───── globals updated in ISR ───── */
volatile uint32_t pulseCount = 0;
float             totalLiters = 0.0;

/* ── values exposed via /stats ── */
uint32_t lastPulses   = 0;
float     lastLiters  = 0.0;
float     lastLpm     = 0.0;

/* ── portable ISR attribute (ESP32 vs everyone else) ── */
#ifdef ARDUINO_ARCH_ESP32
  #define ISR_ATTR IRAM_ATTR
#else
  #define ISR_ATTR
#endif

void ISR_ATTR pulseISR() { pulseCount++; }

/* ─────────────────────────────────── */
void setup() {
  Serial.begin(9600);
  matrix.begin();
  matrix.loadSequence(LEDMATRIX_ANIMATION_HEARTBEAT_LINE);
  matrix.play();

  while (!Serial);

  Serial.print(F("Connecting Wi‑Fi…"));
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.print(F(" OK  IP="));
  Serial.println(WiFi.localIP());

  webServer.begin();

  pinMode(FLOW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), pulseISR, RISING);
}

/* ─────────────────────────────────── */
void advanceMatrixAnim() {
  uint32_t now = millis();
  if (now - lastFrameAdvance >= ANIM_FRAME_INTERVAL) {
    lastFrameAdvance = now;
    matrix.play();  // advance one frame of heartbeat
  }
}

void handleWebRequests() {
  WiFiClient client = webServer.accept();  // non‑blocking
  if (!client) return;

  // read the first request line
  String reqLine = client.readStringUntil('\r');

  /* ── /reset ─────────────────────── */
  if (reqLine.startsWith("POST /reset") || reqLine.startsWith("GET /reset")) {
    noInterrupts();
    pulseCount   = 0;
    totalLiters  = 0.0;
    interrupts();
    client.println(F("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\rcounters reset"));
    Serial.println(F(">>> counters reset via /reset"));
  }
  /* ── /stats ─────────────────────── */
  else if (reqLine.startsWith("GET /stats")) {
    StaticJsonDocument<128> doc;
    doc["pulses"]        = lastPulses;
    doc["delta_liters"]  = lastLiters;
    doc["lpm"]           = lastLpm;
    doc["total_liters"]  = totalLiters;

    String body;  serializeJson(doc, body);
    client.println(F("HTTP/1.1 200 OK"));
    client.println(F("Content-Type: application/json"));
    client.print(F("Content-Length: ")); client.println(body.length());
    client.println();
    client.println(body);
  }
  /* ── 404 ────────────────────────── */
  else {
    client.println(F("HTTP/1.1 404 Not Found\r\n\r\n"));
  }
  client.stop();
}

void publishToTelegraf(const String &payload) {
  WiFiClient client;
  if (!client.connect(TELEGRAF_HOST, TELEGRAF_PORT)) {
    Serial.println(F("Telegraf connection failed"));
    return;
  }

  client.print(F("POST ")); client.print(URL_PATH); client.println(F(" HTTP/1.1"));
  client.print(F("Host: ")); client.println(TELEGRAF_HOST);
  client.println(F("Content-Type: application/json"));
  client.print(F("Content-Length: ")); client.println(payload.length());
  client.println();
  client.println(payload);
  client.stop();
  Serial.print(F("Sent to Telegraf: ")); Serial.println(payload);
}

void loop() {
  advanceMatrixAnim();
  handleWebRequests();

  /* ── timed measurement window ── */
  static uint32_t lastMs = 0;
  uint32_t now = millis();
  if (now - lastMs < MEAS_MS) return;
  uint32_t elapsed = now - lastMs;
  lastMs = now;

  noInterrupts();
  uint32_t pulses = pulseCount;
  pulseCount = 0;
  interrupts();

  float deltaLiters = pulses / PULSES_PER_LITER;
  float lpm         = (deltaLiters * 60000.0) / elapsed;
  totalLiters += deltaLiters;

  /* store for /stats */
  lastPulses  = pulses;
  lastLiters  = deltaLiters;
  lastLpm     = lpm;

  /* Serial debug line (raw pulses) */
  Serial.print(F("pulses:"));    Serial.print(pulses);
  Serial.print(F("  delta_L:")); Serial.print(deltaLiters, 4);
  Serial.print(F("  lpm:"));      Serial.print(lpm, 3);
  Serial.print(F("  total_L:"));  Serial.println(totalLiters, 2);

  /* build JSON for Telegraf */
  StaticJsonDocument<256> doc;
  doc["source"] = "ro_r4wifi";
  JsonObject flow = doc.createNestedObject("flow");
  flow["pulses"]        = pulses;
  flow["delta_liters"]  = deltaLiters;
  flow["lpm"]           = lpm;
  flow["total_liters"]  = totalLiters;

  String payload;
  serializeJson(doc, payload);
  publishToTelegraf(payload);
}


