// ---------------------------------------------------------------------
// WebServer
// ---------
// Example program to show how to create a web server using an ESP32
// ---------------------------------------------------------------------
#include <WiFi.h>
#include <WebServer.h>

// WiFi credentials
String SSID = "MySSID";
String PWD  = "MyPassword";

// Use a global variable for the server since there is only one server
WebServer server(80);

// ---------------------------------------------------------------------
// ConnectWiFi
// -----------
// ---------------------------------------------------------------------
void ConnectWiFi() {
  WiFi.begin(SSID, PWD);
  int loopcnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf("Connecting: time %d, WiFi status = %d, signal = %d\n", loopcnt++, WiFi.status(), WiFi.RSSI());
    delay(1000);
  }
  Serial.printf("Connected: %s\n", WiFi.localIP().toString().c_str());
}

// ---------------------------------------------------------------------
// onRoot
// ------
// ---------------------------------------------------------------------
void onRoot() {
  String response = R"(
    <!DOCTYPE html><html>
      <body>
        <h1>ESP32 web server test</h1>
        <p>John's ESP32 web server test program</p>
      </body>
    </html>
  )";
  server.send(200, "text/html", response);
}

// ---------------------------------------------------------------------
// setup
// -----
// ---------------------------------------------------------------------
void setup() {
  // Connect the serial monitor and wait for it to initialise
  Serial.begin(115200);
  while (!Serial)
    delay(100);
  delay(1000);

  // Connect the WiFi
  ConnectWiFi();

 // Set the paths we will handle
  server.on("/", onRoot);
  // Start the HTTP server
  server.begin();

  // Initialisation complete
  Serial.println("Initialisation complete");
}

// ---------------------------------------------------------------------
// loop
// ----
// ---------------------------------------------------------------------
void loop() {
  static int loop_counter = 0;

  // Check for connections
  server.handleClient();

  // Small delay to avoid using excessive CPU
  delay(100);

  // Every 100 loops (10 secs) print a status message
  loop_counter++;
  if (loop_counter >= 100) {
    Serial.printf("%s, WiFi status = %d, signal = %d\n", WiFi.localIP().toString().c_str(), WiFi.status(), WiFi.RSSI());
    loop_counter = 0;
  }
}
