/*
Original Author: Arvind Sanjeev - DIYhacking.com
Description: Measure the liquid/water flow rate using this code. 
*/

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
    Serial.print("Current Flow Rate: ~ ");
    Serial.print(int(flowMilliLitres * 70.5));  // Print the integer part of the variable
    Serial.println("ml/min");

    // Print the cumulative total of litres flowed since starting
    Serial.print("Water Output This Session: ");
    Serial.print(totalMilliLitres);
    Serial.println("mL");
	  Serial.print("  Total Water Used: ");
	  Serial.print(totalMilliLitres/1000.0);
	  Serial.println(" Litres");

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
