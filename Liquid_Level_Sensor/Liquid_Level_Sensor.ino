/*
Original Author: Unknown - cqrobot.wiki
Original File: http://www.cqrobot.wiki/images/7/7d/Sample_code_for_Arduino.zip
Description: Measure the liquid/water level sensor using this code.
*/

//The byte datatype, stores an 8-bit unsigned number, from 0 to 255
unsigned int ledpin     = 13;
unsigned int inpin      = 7;
unsigned int val;      // assign data-type to val


//The void keyword is used only in function declarations. It indicates that the function is expected to return no information to the function from which it was called.
void setup()
{
    // Initialize a serial connection for reporting values to the host
    Serial.begin(9600);

    // Configures the specified pin to behave either as an input or an output. Here, the status LED line as an output
    pinMode(ledpin,OUTPUT);// set LED pin as “output”
    pinMode(inpin,INPUT);// set button pin as “input”
}

/**
 * Main program loop
 */
void loop()
{
    val=digitalRead(inpin);// read the level value of pin 7 and assign if to val
    Serial.println(val); // print the data from the sensor
    delay(100);
    if (val==LOW) // check if the button is pressed, if yes, turn on the LED
    { digitalWrite(ledpin, LOW);}
    else
    { digitalWrite(ledpin, HIGH);}
}