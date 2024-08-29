/*
Original Author: Arvind Sanjeev - DIYhacking.com
Description: Measure the liquid/water flow rate using this code. 
*/

// Tetris Animation Sketch @ https://docs.arduino.cc/tutorials/uno-r4-wifi/r4-wifi-getting-started/
#include "Arduino_LED_Matrix.h"
#include <stdint.h>

ArduinoLEDMatrix matrix;

const uint32_t frames[][4] = {
  {
    0xe0000000,
    0x0,
    0x0,
    66
  },
  {
    0x400e0000,
    0x0,
    0x0,
    66
  },
  {
    0x400e0,
    0x0,
    0x0,
    66
  },
  {
    0x40,
    0xe000000,
    0x0,
    66
  },
  {
    0x3000000,
    0x400e000,
    0x0,
    66
  },
  {
    0x3003000,
    0x400e,
    0x0,
    66
  },
  {
    0x3003,
    0x4,
    0xe00000,
    66
  },
  {
    0x3,
    0x300000,
    0x400e00,
    66
  },
  {
    0x0,
    0x300300,
    0x400e00,
    66
  },
  {
    0x1c000000,
    0x300,
    0x30400e00,
    66
  },
  {
    0x401c000,
    0x0,
    0x30430e00,
    66
  },
  {
    0x401c,
    0x0,
    0x430e30,
    66
  },
  {
    0x4,
    0x1c00000,
    0x430e30,
    66
  },
  {
    0x0,
    0x401c00,
    0x430e30,
    66
  },
  {
    0x800000,
    0x401,
    0xc0430e30,
    66
  },
  {
    0x800800,
    0x0,
    0x405f0e30,
    66
  },
  {
    0x800800,
    0x80000000,
    0x470ff0,
    66
  },
  {
    0x800800,
    0x80080000,
    0x470ff0,
    66
  },
  {
    0x800,
    0x80080080,
    0x470ff0,
    66
  },
  {
    0x38000000,
    0x80080080,
    0x8470ff0,
    66
  },
  {
    0x10038000,
    0x80080,
    0x8478ff0,
    66
  },
  {
    0x10038,
    0x80,
    0x8478ff8,
    66
  },
  {
    0x700010,
    0x3800080,
    0x8478ff8,
    66
  },
  {
    0x400700,
    0x1003880,
    0x8478ff8,
    66
  },
  {
    0x400,
    0x70001083,
    0x88478ff8,
    66
  },
  {
    0xf000000,
    0x40070081,
    0x87f8ff8,
    66
  },
  {
    0xf000,
    0x400f1,
    0x87f8ff8,
    66
  },
  {
    0x8000000f,
    0xc1,
    0xf7f8ff8,
    66
  },
  {
    0xc0080000,
    0xf00081,
    0xc7ffff8,
    66
  },
  {
    0x400c0080,
    0xf81,
    0x87fcfff,
    66
  },
  {
    0x3400c0,
    0x8000081,
    0xf87fcfff,
    66
  },
  {
    0x20200340,
    0xc008081,
    0xf87fcfff,
    66
  },
  {
    0x38220200,
    0x3400c089,
    0xf87fcfff,
    66
  },
  {
    0x38220,
    0x2003408d,
    0xf8ffcfff,
    66
  },
  {
    0x86100038,
    0x220240bd,
    0xf8ffcfff,
    66
  },
  {
    0xec186100,
    0x38260ad,
    0xfbffcfff,
    66
  },
  {
    0x3ec186,
    0x100078af,
    0xfaffffff,
    66
  },
  {
    0x114003ec,
    0x186178af,
    0xfaffffff,
    66
  },
  {
    0x3b411400,
    0x3ec1febf,
    0xfaffffff,
    66
  },
  {
    0x143b411,
    0x4ec3febf,
    0xfbffffff,
    66
  },
  {
    0xc040143b,
    0x4fd7febf,
    0xfbffffff,
    66
  },
  {
    0xc60c0439,
    0x4ff7ffff,
    0xfbffffff,
    66
  },
  {
    0x33c60f9,
    0x4ff7ffff,
    0xffffffff,
    66
  },
  {
    0x3cbc33ff,
    0x4ff7ffff,
    0xffffffff,
    66
  },
  {
    0x8ffbff,
    0x7ff7ffff,
    0xffffffff,
    66
  },
  {
    0xf0cffbff,
    0xfff7ffff,
    0xffffffff,
    66
  },
  {
    0xfe1fffff,
    0xffffffff,
    0xffffffff,
    66
  },
  {
    0xffffffff,
    0xffffffff,
    0xffffffff,
    66
  },
  {
    0x7fffffff,
    0xffffffff,
    0xfffff7ff,
    66
  },
  {
    0x3fe7ffff,
    0xffffffff,
    0xff7ff3fe,
    66
  },
  {
    0x1fc3fe7f,
    0xfffffff7,
    0xff3fe1fc,
    66
  },
  {
    0xf81fc3f,
    0xe7ff7ff3,
    0xfe1fc0f8,
    66
  },
  {
    0x500f81f,
    0xc3fe3fe1,
    0xfc0f8070,
    66
  },
  {
    0x500f,
    0x81fc1fc0,
    0xf8070020,
    66
  },
  {
    0x5,
    0xf80f80,
    0x70020000,
    66
  },
  {
    0x5,
    0xa80880,
    0x50020000,
    600
  },
  {
    0xd812,
    0x41040880,
    0x50020000,
    200
  },
  {
    0x5,
    0xa80880,
    0x50020000,
    0xFFFFFFFF
  }
};

//The byte datatype, stores an 8-bit unsigned number, from 0 to 255
byte statusLed    = 13;
byte sensorInterrupt = 0;  // value set to 0 which equals digital pin 2 - (makes using a breadboard unnecessary)
byte sensorPin       = 2;

// This flow sensor model outputs approximately 5880 pulses, but is that over a minute otherwise over a second its 5880/60 = 98 Hz square wave which has a period of 1/98 = 10.2 milliseconds.
float calibrationFactor = 48.95;

volatile byte pulseCount;


float flowRate;
float totalMilliLitres;
unsigned int flowMilliLitres; //The conversion "unsigned" int/long won't store negative numbers and are helpful with interrupt service routines sections of code that are associated with interrupts.
// unsigned long totalMilliLitres;

unsigned long oldTime;

//The void keyword is used only in function declarations. It indicates that the function is expected to return no information to the function from which it was called.
void setup()
{
  // Initialize a serial connection for reporting values to the host
  Serial.begin(9600);
  matrix.begin();

  const uint32_t happy[] = {
    0x19819,
    0x80000001,
    0x81f8000
  };

  const uint32_t heart[] = {
    0x3184a444,
    0x44042081,
    0x100a0040
  };

  // Configures the specified pin to behave either as an input or an output. Here, the status LED line as an output
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
  
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0; // 1000mL should be approx 5880 pulses
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0 and is configured to trigger on a FALLING state change (transition from HIGH state to LOW state)
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
}

/**
 * Main program loop
 */
void loop() {

  matrix.loadFrame(happy);
  delay(500);

  matrix.loadFrame(heart);
  delay(500);
   
   if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensorInterrupt);
        
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    
    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    oldTime = millis();
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 65.0) * 1000;
    
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
      
    unsigned int frac;

    // Print the flow rate for this second in litres / minute
    Serial.print("Water Flow Rate: ~ ");
    Serial.print(int(flowMilliLitres * 70.5));  // Print the integer part of the variable
    Serial.println("ml/min");

    // Print the cumulative total of litres flowed since starting
    Serial.print("Water Output: ");
    Serial.print(totalMilliLitres);
    Serial.println("mL");
	  Serial.print("  Total Liters of Water Used: ");
	  Serial.println(totalMilliLitres/1000.0);

    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }
}

//   Insterrupt Service Routine
void pulseCounter()
{
  // Increment the pulse counter by 1
  pulseCount++;
}