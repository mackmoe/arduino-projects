//face drip
#include "Arduino_LED_Matrix.h"
#include <stdint.h>

ArduinoLEDMatrix matrix;

// Pin to which the flow sensor is connected
const int flowSensorPin = 2;

// Variables to store the pulse count and flow rate
volatile int pulseCount = 0;
float flowRate = 0.0;
float flowMilliLitres = 0;
float totalMilliLitres  = 0;
unsigned long oldTime = 0;

// This flow sensor model outputs approximately 5880 pulses, but is that over a minute otherwise over a second its 5880/60 = 98 Hz square wave which has a period of 1/98 = 10.2 milliseconds.
float calibrationFactor = 48.95;

void setup() {
  // Initialize the serial communication
  Serial.begin(9600);
  
  // load frames at runtime, without stopping the refresh
  matrix.loadSequence(LEDMATRIX_ANIMATION_STARTUP);
  matrix.begin();
  matrix.play(true);
  
  // Set the flow sensor pin as an input
  pinMode(flowSensorPin, INPUT_PULLUP);

  // Attach an interrupt to the flow sensor pin
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, FALLING);
}

void loop() {
  // Time interval for calculation (e.g., 1 second)
  unsigned long currentTime = millis();

  if (currentTime - oldTime > 1000) { // Calculate every second
    detachInterrupt(digitalPinToInterrupt(flowSensorPin));

    // Calculate the flow rate in liters per hour (L/h)
    // Formula: flowRate = (Pulse frequency / 7.5) in L/h
    flowRate = ((1000.0 / (currentTime - oldTime)) * pulseCount) / calibrationFactor;

    // Convert flow rate to milliliters per second (mL/s)
    flowMilliLitres = (flowRate / 60) * 1000;
    unsigned int frac;

    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
   
    // Print the flow rate
    Serial.print("Current Flow Rate: ~ ");
    Serial.print(int(flowMilliLitres * 70.5));  // Print the integer part of the variable
    Serial.println("mL/min");
    // Print the cumulative total of litres flowed since starting
    Serial.print("Water Output This Session: ");
    Serial.print(totalMilliLitres);
    Serial.println("mL");
    // Print the cumulative total of litres flowed since starting
    Serial.print("Total Water Used: ");
	  Serial.print(totalMilliLitres/1000.0);
	  Serial.println(" Litres");

    pulseCount = 0;   // Reset the pulse count
    oldTime = currentTime;

    // Reattach the interrupt for the next interval
    attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, FALLING);
  }
}

// Function to count the pulses from the sensor
void pulseCounter() {
  pulseCount++;
}
