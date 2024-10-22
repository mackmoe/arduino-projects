#include <SPI.h>

// Water flow sensor connection
const int flowSensorPin = 2;  // Digital pin for water flow sensor
volatile int pulseCount = 0;


// Variables for flow calculation
float calibrationFactor = 4.5;  // Calibration factor for the sensor (Adjust as needed)
float flowRate = 0;
float totalLiters = 0;
unsigned long previousMillis = 0;
const unsigned long interval = 3600000; // 1 hour in milliseconds


// Interrupt Service Routine for counting pulses
void pulseCounter() {
  pulseCount++;
}

void setup() {
  // Start Serial communication
  Serial.begin(9600);
  
  // Setup flow sensor input and attach interrupt
  pinMode(flowSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, RISING);
  
  Serial.println("Time (hours), Flow Rate (Gal/min), Total Volume (Gallons)");
}

void loop() {
  unsigned long currentMillis = millis();

  // Calculate flow rate every hour
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    // Disable the interrupt while calculating
    detachInterrupt(digitalPinToInterrupt(flowSensorPin));
    
    // Calculate the flow rate in L/min
    flowRate = ((float)pulseCount / calibrationFactor) / 60;
    totalLiters += flowRate;
    
    // Send data to the Serial port
    Serial.print(currentMillis / 3600000); // Time in hours
    Serial.print(", ");
    Serial.print(flowRate);
    Serial.print(", ");
    Serial.println(totalLiters);
    
    // Reset pulse count
    pulseCount = 0;
    
    // Re-enable the interrupt
    attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, RISING);
  }
}