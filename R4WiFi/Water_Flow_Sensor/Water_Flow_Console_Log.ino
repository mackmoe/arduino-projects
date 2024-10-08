//face drip LIBs, grafx first!
#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>
ArduinoLEDMatrix matrix; 


//wireless libs
#include <SPI.h>
#include <WiFiS3.h>  // Use WiFiS3 for Arduino Uno R4 WiFi
const char* ssid = "TryMe";
const char* pass = "tyw7c4u6egfr";
int status = WL_IDLE_STATUS; // set WiFi radio's status var
// Create a WiFi server on port 80
WiFiServer server(80);


// This flow sensor model outputs approximately 5880 pulses, but is that over a minute otherwise over a second its 5880/60 = 98 Hz square wave which has a period of 1/98 = 10.2 milliseconds.
float calibrationFactor = 48.95;
// Pin to the flow sensor
const int flowSensorPin = 2;
// Variables to store the pulse count and flow rate
volatile int pulseCount = 0;
float flowRate = 0.0;
float flowMilliLitres = 0;
float totalMilliLitres  = 0;
unsigned long oldTime = 0;


void setup() {
  // Initialize the serial communication
  Serial.begin(9600);

  // load face drip frames at runtime
  matrix.begin();
  matrix.loadSequence(LEDMATRIX_ANIMATION_STARTUP);
  matrix.play(true);

  // attempt to connect to the WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  // Print wireless connection details
  printWifiData();

  // Set the flow sensor pin as an input
  pinMode(flowSensorPin, INPUT_PULLUP);

  // Attach an interrupt to the flow sensor pin
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, FALLING);

  // Start the web server
  server.begin();
}

void faceDrip() {
  matrix.beginDraw();

  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(30);

  // add the text
  const char text[] = "    Arduino WaTeR FLoW MoNiToR    ";
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text);
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
}

void loop() {
  faceDrip(); // Makezit scroll!
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
    Serial.print(int(flowRate * 70.5));  // Print the integer part of the variable
    Serial.println("mL/min");
    // Print the cumulative total of litres flowed since starting
    Serial.print("Water Output This Session: ");
    Serial.print(flowMilliLitres);
    Serial.println("mL");
    // Print the cumulative total of litres flowed since starting
    Serial.print("Total Water Used: ");
	  Serial.print(totalMilliLitres);
	  Serial.println(" Litres");

    pulseCount = 0;   // Reset the pulse count
    oldTime = currentTime;

    // Reattach the interrupt for the next interval
    attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, FALLING);
  }
  
  // Check if a client has connected
  WiFiClient client = server.available();  
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    if (request.indexOf("/status") != -1) {
      // Send flow data as a response
      client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
      client.print("Approx current Flow Rate: ");
      client.print(int(flowRate * 70.5));
      client.println("mL/min");
      client.print("Water Output This Session: ");
      client.print(flowMilliLitres);
      client.println("mL");
      client.print("Total Water Used: ");
      client.print(totalMilliLitres);
      client.println(" Litres");
    } else {
      // Send the web page as a response
      client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
      client.print("<html><head><body>");
      client.print("<h1>Jack'd Water Flow Monitor Refresh'd Output</h1>");
      client.print("<p>Current Flow Rate: <span id='flowRate'>0</span> mL/s</p>"
      "<p>Water Output This Session: <span id='flowMilliLitres'>0</span> mL/s</p>"
      "<p>Total Water Used: <span id='totalMilliLitres'>0</span> L</p>"
      );
      client.print("<script>setInterval(function() { fetch('/status').then(response => response.text()).then(data => { document.getElementById('flowRate','flowMilliLitres','totalMilliLitres').innerText = data; }); }, 1000);</script>");
      client.print("</body></head></html>");
    }
    delay(1);
    client.stop();
  }
}

void makeitScroll() {
  matrix.begin();

  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(70);

  const char text[] = "    ArDuiNo WaTeR FLoW MoNiToR   ";
  matrix.textFont(Font_4x6);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text);
  matrix.endText(SCROLL_LEFT);

  matrix.endDraw();
}

// The wireless connection details
void printWifiData() {
  // print the SSID of the network you're attached to:
  Serial.print("Connected to: ");
  Serial.println(WiFi.SSID());

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("WiFi Signal Strength: ");
  Serial.println(rssi);

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("Arduino IP: ");
  Serial.println(ip);
  
  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC Address: ");
  printMacAddress(mac);
}

// push/pop values to print 
void printMacAddress(byte mac[]) {
  for (int i = 0; i < 6; i++) {
    if (i > 0) {
      Serial.print(":");
    }
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
  }
  Serial.println();
}

// Function to count the pulses from the sensor
void pulseCounter() {
  pulseCount++;
}
