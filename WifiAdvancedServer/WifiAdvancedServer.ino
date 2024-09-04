#include "WiFi.h"
#include "WiFiS3.h"

#include "arduino_secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password 

int status = WL_IDLE_STATUS;

WiFiServer server(80); // Creates a server on port 80

void setup() {
  Serial.begin(9600);      // initialize serial communication

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);
    // start wifi con
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds
    delay(10000);
  }
  // start http server
  server.begin();                           // start the web server on port 80
  printWifiStatus();                        // you're connected now, so print out the status
}

void printWifiStatus() {
  Serial.print("Your Board's Wirelessly Connected to: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("Your Board's IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  
  Serial.print("The Arduino WebServer is ready at http://");
  Serial.println(ip);
}


void loop() {
  WiFiClient client = server.available();   // listen for incoming clients
  if (client) {
    Serial.println("New Client Connected"); // print each open connection to console
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close"); 
    client.println();
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<head><title>Arduino Console Output</title></head>");
    client.println("<body>");
    client.println("<h1>Console Output</h1>");
    client.println("<pre id='consoleOutput'></pre>");
    client.println("<script>");
    client.println("setInterval(function() {");
    client.println("  fetch('/console').then(response => response.text()).then(data => {");
    client.println("    document.getElementById('consoleOutput').innerText = data;");
    client.println("  });");
    client.println("}, 1000);");
    client.println("</script>");
    client.println("</body>");
    client.println("</html>");

    // give the web browser time to receive the data
    delay(1);

    // Close the connection after sending the response
    client.stop();
    Serial.println("Client Disconnected"); // print each connection closed to console
  }
  
  WiFiClient consoleClient = server.available(); // Handle '/console' endpoint to send the current serial output
  if (consoleClient) {
    // If the client requests the "/console" endpoint
    if (consoleClient.find("GET /console")) {
      // Send the latest Serial output
      consoleClient.println("HTTP/1.1 200 OK");
      consoleClient.println("Content-Type: text/plain");
      consoleClient.println("Connection: close"); 
      consoleClient.println();
      
      // Send the data from the Serial buffer
      while (Serial.available()) {
        consoleClient.write(Serial.read());
      }
      consoleClient.stop();
    }
  }
}

