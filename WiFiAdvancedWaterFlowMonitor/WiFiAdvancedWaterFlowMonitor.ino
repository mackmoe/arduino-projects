#include "WiFiS3.h"
#include "WiFiServer.h"
#include "WiFiUdp.h"
#include "NTPClient.h"

#include "wifisecret.h" // Wi-Fi network details
char ssid[] = SECRET_SSID;        // Replace with your WiFi network name
char pass[] = SECRET_PASS; // Replace with your WiFi password

// NTP Client setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -18000, 60000); // NTP server, UTC offset in seconds, update interval

int status = WL_IDLE_STATUS;
WiFiServer server(80); // Web server running on port 80

// Flow sensor configuration
const int flowSensorPin = 2; // Flow sensor connected to digital pin 2
volatile int pulseCount = 0;
float flowRate = 0.0;
float totalLiters = 0.0;
unsigned long previousMillis = 0;
const unsigned long interval = 1000; // Update interval in milliseconds (1 second)

// Calibration factor for the specific sensor (YF-S201 uses 4.5)
const float calibrationFactor = 4.5;

// Data for daily water usage (24 hourly readings)
float hourlyUsage[24] = {0};

// Interrupt service routine to count the pulses
void pulseCounter() {
  pulseCount++;
}

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Set up the flow monitor stuff
  pinMode(flowSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, RISING);

  // Initialize Wi-Fi connection
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Wireless Network: ");
    Serial.println(ssid);
    // start wifi con
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds
    delay(10000);
  }
  printWifiStatus();

  // Start the NTP client
  timeClient.begin();
  timeClient.update();

  // Start the server
  server.begin();
  Serial.println("Web server started");

  // Initialize hourly usage array
  memset(hourlyUsage, 0, sizeof(hourlyUsage));
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("Water Flow Usage up on http://");
  Serial.println(ip);
}

void loop() {
  unsigned long currentMillis = millis();

  // Update NTP time periodically
  timeClient.update();

  // Calculate flow rate and accumulate total usage every second
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Calculate the flow rate in liters per minute
    flowRate = ((pulseCount / calibrationFactor) / 60.0);
    totalLiters += (flowRate / 60.0); // Convert flow rate to liters per second

    // Get the current hour from NTP time
    int currentHour = timeClient.getHours();

    // Update hourly usage data
    if (currentHour >= 0 && currentHour < 24) {
      hourlyUsage[currentHour] += (flowRate / 60.0);
    }

    // Reset pulse count
    pulseCount = 0;
  }

  // Handle client requests
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client connected");
    // Wait until the client sends some data
    while (!client.available()) {
      delay(1);
    }

    // Read the request
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();

    // Prepare the response
    String response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
    response += "<!DOCTYPE HTML><html><link rel=https://static-00.iconduck.com/assets.00/apps-arduino-icon-256x256-mp2raho4.png></link><head><title>Water Usage Monitor</title>";
    response += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script></head><body>";
    response += "<h2>Daily Water Usage Monitor</h2>";
    response += "<canvas id='usageChart' width='400' height='200'></canvas>";
    response += "<script>";
    response += "const ctx = document.getElementById('usageChart').getContext('2d');";
    response += "const usageData = { labels: [";

    // Add labels for each hour (0-23)
    for (int i = 0; i < 24; i++) {
      response += "'" + String(i) + ":00'";
      if (i < 23) response += ",";
    }

    response += "], datasets: [{ label: 'Water Usage (G)', data: [";

    // Add hourly water usage data
    for (int i = 0; i < 24; i++) {
      response += String(hourlyUsage[i]);
      if (i < 23) response += ",";
    }

    response += "], backgroundColor: 'rgba(75, 192, 192, 0.2)', borderColor: 'rgba(75, 192, 192, 1)', borderWidth: 1 }]};";
    response += "const config = { type: 'bar', data: usageData, options: { scales: { y: { beginAtZero: true } } } };";
    response += "const usageChart = new Chart(ctx, config);</script></body></html>";

    // Send the response to the client
    client.print(response);
    client.stop();
    Serial.println("Client disconnected");
  }
}
