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
constexpr float PULSES_PER_LITER = 3695.0;   // calibrate (lpm) for your SEN‑HZ06W - 3690 +≈ 3.3 %
constexpr uint32_t MEAS_MS = 1000;           // publish cadence (ms)

/* ─────  control server (/reset) ───── */
WiFiServer ctrlServer(8080);

/* ───── LED‑matrix ───── */
ArduinoLEDMatrix matrix;
constexpr uint32_t ANIM_FRAME_INTERVAL = 3000; // ms per frame of heartbeat
uint32_t lastFrameAdvance = 0;

/* ───── globals updated in ISR ───── */
volatile uint32_t pulseCount = 0;
float             totalLiters = 0.0;

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
  matrix.play();                // start heartbeat immediately

  while (!Serial);

  Serial.print(F("Connecting Wi‑Fi…"));
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.print(F(" OK  IP="));
  delay(1000);
  Serial.println(WiFi.localIP());

  ctrlServer.begin();

  pinMode(FLOW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), pulseISR, RISING);
}

/* ─────────────────────────────────── */
void advanceMatrixAnim() {
  uint32_t now = millis();
  if (now - lastFrameAdvance >= ANIM_FRAME_INTERVAL) {
    lastFrameAdvance = now;
    matrix.play();              // advance one frame of HEARTBEAT
  }
}

void loop() {
  advanceMatrixAnim();

  /* 0️⃣  Check for /reset requests (non‑blocking) */
  WiFiClient ctrl = ctrlServer.accept();
  if (ctrl) {
    String reqLine = ctrl.readStringUntil('\r');
    if (reqLine.startsWith("POST /reset") || reqLine.startsWith("GET /reset")) {
      noInterrupts();
      pulseCount  = 0;
      totalLiters = 0.0;
      interrupts();
      ctrl.println(F("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\rcounters reset"));
      Serial.println(F(">>> counters reset via /reset"));
    } else {
      ctrl.println(F("HTTP/1.1 404 Not Found\r\n\r\n"));
    }
    ctrl.stop();
  }

  /* 1️⃣  Publish every MEAS_MS */
  static uint32_t lastMs = 0;
  uint32_t now = millis();
  if (now - lastMs < MEAS_MS) return;
  uint32_t elapsed = now - lastMs;
  lastMs = now;

  noInterrupts();
  uint32_t pulses = pulseCount; pulseCount = 0;
  interrupts();

  float liters = pulses / PULSES_PER_LITER;
  float lpm    = (liters * 60000.0) / elapsed;
  totalLiters += liters;

  StaticJsonDocument<256> doc;
  doc["source"] = "ro_r4wifi";
  JsonObject flow = doc.createNestedObject("flow");
  flow["lpm"]          = lpm;
  flow["total_liters"] = totalLiters;
  flow["delta_liters"] = liters;

  String payload;
  serializeJson(doc, payload);

  WiFiClient client;
  if (client.connect(TELEGRAF_HOST, TELEGRAF_PORT)) {
    client.print(F("POST ")); client.print(URL_PATH); client.println(F(" HTTP/1.1"));
    client.print(F("Host: ")); client.println(TELEGRAF_HOST);
    client.println(F("Content-Type: application/json"));
    client.print(F("Content-Length: ")); client.println(payload.length());
    client.println();
    client.println(payload);
    client.stop();
    Serial.print(F("Sent: ")); Serial.println(payload);
  } else {
    Serial.println(F("Telegraf connection failed"));
  }
}