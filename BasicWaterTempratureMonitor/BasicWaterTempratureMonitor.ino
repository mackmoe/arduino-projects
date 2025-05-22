#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into digital pin 2 on the Uno
#define ONE_WIRE_BUS 2

// Setup a OneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our OneWire reference to Dallas Temperature library
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);
  while(!Serial) { /* wait for Serial on Leonardo/Micro */ }

  // Start up the library
  sensors.begin();

  Serial.println(F("DS18B20 Water‑Temp Monitor"));
}

void loop() {
  // Request a temperature measurement
  sensors.requestTemperatures();
  
  // Fetch temperature in °C from the first (and only) sensor
  float tempC = sensors.getTempCByIndex(0);

  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println(F("Error: Could not read temperature data"));
  } else {
    // Print Celsius
    Serial.print(F("Water Temp: "));
    Serial.print(tempC, 2);
    Serial.print(F(" °C  "));
    
    // Also print Fahrenheit if you like:
    float tempF = sensors.toFahrenheit(tempC);
    Serial.print(F("("));
    Serial.print(tempF, 2);
    Serial.println(F(" °F)"));
  }

  delay(1000);
}
