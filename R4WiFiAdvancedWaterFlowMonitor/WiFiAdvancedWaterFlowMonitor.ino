/*
  RO water-flow monitor – UNO R4 WiFi  (with /reset endpoint)
*/
#include <WiFiS3.h>
#include <ArduinoJson.h>
#include <Arduino_LED_Matrix.h>
#include "arduino_secrets.h"

/* ───── Wi-Fi & Telegraf ───── */
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
const IPAddress TELEGRAF_HOST(xxx, xxx, xxx, x);
constexpr uint16_t TELEGRAF_PORT = 8126;
constexpr char     URL_PATH[]    = "/ro-water-monitor";

/* ───── Flow sensor ───── */
constexpr byte  FLOW_PIN = 2;
constexpr float PULSES_PER_LITER = 4020.0;
constexpr uint32_t MEAS_MS = 5'000;

/* ─────  control server ───── */
WiFiServer ctrlServer(8080);           // <-- listen on port 8080

/* ── thresholds with hysteresis ── */
const float DRIP_MAX  = 0.50;   // below → DRIP
const float WAVE_MIN  = 0.70;   // above → WAVE


// ── live-flow display states ──────────────
ArduinoLEDMatrix matrix;
enum DisplayMode : uint8_t { MODE_NONE, MODE_DRIP, MODE_WAVE };

DisplayMode currentMode = MODE_NONE;
DisplayMode lastDisplayedMode = MODE_NONE;

uint32_t lastChangeMs = 0;
uint32_t lastFrameAdvance = 0;

const uint16_t ANIM_FRAME_INTERVAL = 3000; // ms between animation steps
const uint32_t DISPLAY_INTERVAL = 3000; // 3 seconds

void updateFlowDisplay(float lpm) {
  uint32_t now = millis();

  if (now - lastChangeMs < DISPLAY_INTERVAL) {
    return; // Wait for 5 seconds before evaluating again
  }

  DisplayMode desired = MODE_NONE;
  if (lpm < DRIP_MAX) {
    desired = MODE_DRIP;
  } else if (lpm > WAVE_MIN) {
    desired = MODE_WAVE;
  }

  // Only update if the mode actually needs to change
  if (desired != currentMode) {
    currentMode = desired;
    lastChangeMs = now;

    if (currentMode != lastDisplayedMode) {
      lastDisplayedMode = currentMode;

      switch (currentMode) {
        case MODE_DRIP:
          matrix.loadSequence(LEDMATRIX_ANIMATION_HEARTBEAT_LINE);
          matrix.play();
          break;

        case MODE_WAVE:
          matrix.loadSequence(LEDMATRIX_ANIMATION_AUDIO_WAVEFORM);
          matrix.play();
          break;

        case MODE_NONE:
        default:
          matrix.clear();
          break;
      }
    }
  }
}

/* ───── globals updated in ISR ───── */
volatile uint32_t pulseCount = 0;
float             totalLiters = 0.0;

void pulseISR() { pulseCount++; }

/* ─────────────────────────────────── */

void setup() {
  Serial.begin(9600);
  matrix.begin();
  while (!Serial);                     // wait for USB enum (optional)

  /* connect Wi-Fi */
  Serial.print(F("Connecting Wi-Fi…"));
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  delay(1000);
  Serial.print(F(" OK  IP="));
  Serial.println(WiFi.localIP());

  ctrlServer.begin();                  // <-- start /reset listener

  pinMode(FLOW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), pulseISR, RISING);
}

/* ─────────────────────────────────── */

void loop() {
  uint32_t now = millis();  // only declared once
  if (now - lastFrameAdvance >= ANIM_FRAME_INTERVAL) {
    lastFrameAdvance = now;
    matrix.play();  // plays 1 frame
  }

  /* 0️⃣  check for /reset ------------------------------------------------ */
  WiFiClient ctrl = ctrlServer.accept();   // non-blocking
  if (ctrl) {
    String reqLine = ctrl.readStringUntil('\r');   // e.g. "POST /reset ..."
    if (reqLine.startsWith("POST /reset") || reqLine.startsWith("GET /reset")) {
      noInterrupts();
      pulseCount  = 0;
      totalLiters = 0.0;
      interrupts();
      ctrl.println(F("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nreset"));
      Serial.println(F(">>> counters reset via /reset"));
    } else {
      ctrl.println(F("HTTP/1.1 404 Not Found\r\n\r\n"));
    }
    ctrl.stop();
  }

  /* 1️⃣  every "MEAS_MS" (s) send metrics ---------------------------------------- */
  static uint32_t lastMs = 0;
  if (now - lastMs < MEAS_MS) return;
  uint32_t elapsed = now - lastMs;
  lastMs = now;

  noInterrupts();
  uint32_t pulses = pulseCount; pulseCount = 0;
  interrupts();

  float liters = pulses / PULSES_PER_LITER;
  float lpm = (liters * 60000.0) / elapsed;
  totalLiters += liters;

  updateFlowDisplay(lpm);
  
  StaticJsonDocument<256> doc;
  doc["source"] = "ro_r4wifi";
  JsonObject flow = doc.createNestedObject("flow");
  flow["lpm"]          = lpm;
  flow["total_liters"] = totalLiters;
  flow["delta_liters"] = liters;

  String payload;  serializeJson(doc, payload);

  WiFiClient client;
  if (client.connect(TELEGRAF_HOST, TELEGRAF_PORT)) {
    client.print(F("POST ")); client.print(URL_PATH); client.println(F(" HTTP/1.1"));
    client.print(F("Host: ")); client.println(TELEGRAF_HOST);
    client.println(F("Content-Type: application/json"));
    client.print(F("Content-Length: ")); client.println(payload.length());
    client.println(); client.println(payload);
    client.stop();
    Serial.print(F("Sent: ")); Serial.println(payload);
  } else {
    Serial.println(F("Telegraf connection failed"));
  }
}
