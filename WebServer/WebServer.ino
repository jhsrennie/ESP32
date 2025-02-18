// ---------------------------------------------------------------------
// WebServer
// ---------
// Example program to show how to create a web server using an ESP32
// ---------------------------------------------------------------------
#include <WiFi.h>
#include <WebServer.h>

#define LED_BUILTIN 2

// WiFi credentials
String SSID = "MySSID";
String PWD  = "MyPassword";

// Use a global variable for the server since there is only one server
WebServer server(80);

// ---------------------------------------------------------------------
// ConnectWiFi
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
// This called when the root directory is requested
// ---------------------------------------------------------------------
void onRoot() {
  // Check if the form was submitted
  String formtext = "";
  if (server.hasArg("formtext"))
    formtext = server.arg("formtext");

  // Construct the response
  String response = R"(
    <!DOCTYPE html><html>
      <body>
        <h1>ESP32 web server test</h1>
        <p>John's ESP32 web server test program</p>
        <h3>Turn the on board LED:</h3>
        <ul>
          <li><a href="/on">On</a></li>
          <li><a href="/off">Off</a></li>
        </ul>
        <h3>Example form</h3>
        <form action="/" method="GET">
          <p>Type some text:</p>
          <input type="text" name="formtext">
          <input type="submit" value="Click Me">
        </form>)";

  if (formtext != "")
    response += "<hr/><p>You typed \"" + formtext + "\"</p>";

  response += "</body></html>";

  // Send the response
  server.send(200, "text/html", response);
}

// ---------------------------------------------------------------------
// onOn and onOff
// Turn the on board LED on and off
// ---------------------------------------------------------------------

void onOn() {
  // Turn the LED on
  digitalWrite(LED_BUILTIN, HIGH);
  // And redirect to the home page
  onRoot();
}

void onOff() {
  // Turn the LED off
  digitalWrite(LED_BUILTIN, LOW);
  // And redirect to the home page
  onRoot();
}

// ---------------------------------------------------------------------
// setup
// ---------------------------------------------------------------------
void setup() {
  // Connect the serial monitor and wait for it to initialise
  Serial.begin(115200);
  delay(1000);

  // Set the LED GPIO as output
  pinMode(LED_BUILTIN, OUTPUT);

  // Connect the WiFi
  ConnectWiFi();

 // Set the paths we will handle
  server.on("/", onRoot);
  server.on("/on", onOn);
  server.on("/off", onOff);
  // Start the HTTP server
  server.begin();

  // Initialisation complete
  Serial.println("Initialisation complete");
}

// ---------------------------------------------------------------------
// loop
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
