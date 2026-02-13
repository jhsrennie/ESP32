//----------------------------------------------------------------------
// Arduino HTTP example
// ====================
// This shows how to use the Arduino IDE web server class.
// All the code is in Server.cpp and Server.h. Just copy these files
// into your sketch folder and call ServerSetup() from setup function,
// then call ServerLoop() from your loop function.
//----------------------------------------------------------------------
#include "Server.h"
// WiFi code
#include "../wifi.cpp"

//----------------------------------------------------------------------
// setup
//----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting HTTP example");

  // Connect the WiFi
  ConnectWiFi(WIFI_SSID, WIFI_PASS, false);

  // Setup the web server
  ServerSetup();

  // Done
  Serial.println("Started HTTP Example");
  Serial.print("Connect to http://");
  Serial.print(WiFi.localIP());
  Serial.println("/ to view the web site");
}

//----------------------------------------------------------------------
// loop
//----------------------------------------------------------------------
void loop() {
  // Allow the web server to process connections
  ServerLoop();

  // Short delay so we don't hog the CPU
  delay(100);
}