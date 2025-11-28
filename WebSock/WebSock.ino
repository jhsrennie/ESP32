//----------------------------------------------------------------------
// WebSock.ino
// -----------
// Simple example WebSockets sketch
//----------------------------------------------------------------------
#include <WiFi.h>

// Replace with your network credentials
const char* ssid = "MySSID";
const char* password = "MyPassword";

// Function from websock.cpp
void initWebSocket();

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(WIFI_PS_NONE);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  int loopcnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf("Connecting: time %d, WiFi status = %d, signal = %d\n", loopcnt++, WiFi.status(), WiFi.RSSI());
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

// setup
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("WebSock test app");

  initWiFi();
  initWebSocket();
}

// loop
// The server runs as a separate task so just use a delay to avoid
// hogging the CPU.
void loop() {
  delay(1000);
}