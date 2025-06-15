#include <SPI.h>

const int flowSensorPin = 2;
volatile int pulseCount = 0;

float calibrationFactor = 7.5;       // For flow rate in L/min
float pulsesPerLiter = 450.0;        // For total liters
float flowRate = 0;
float totalLiters = 0;

unsigned long previousMillis = 0;
const unsigned long interval = 1000; // 1 second
unsigned long pulsesPrevious = 0;

void pulseCounter() {
  pulseCount++;
}

void setup() {
  Serial.begin(9600);
  pinMode(flowSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, RISING);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    detachInterrupt(digitalPinToInterrupt(flowSensorPin));

    float pulsesThisPeriod = pulseCount - pulsesPrevious;

    // L/min = (pulses/sec ÷ calibrationFactor) × 60
    flowRate = (pulsesThisPeriod / calibrationFactor) * 60.0;

    // Total liters = (pulses ÷ pulsesPerLiter)
    float litersThisPeriod = pulsesThisPeriod / pulsesPerLiter;
    totalLiters += litersThisPeriod;

    Serial.print("{\"source\":\"arduino-uno-ro\",");
    Serial.print("\"flow_rate\":");
    Serial.print(flowRate, 2);
    Serial.print(",\"total_liters\":");
    Serial.print(totalLiters, 3);
    Serial.println("}");

    pulsesPrevious = pulseCount;
    attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, RISING);
  }
}
