// ---------------------------------------------------------------------
// TestRHSWiFi
// -----------
// Demonstrate how to use the CRHSWiFi class
// ---------------------------------------------------------------------
#include "RHSWiFi.h"

#define MY_SSID "MySSID"
#define MY_PWD  "MyPassword"

// Typically you declare the single wifi object as a global
CRHSWiFi wifi;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(1000);
  Serial.println("Starting WiFi test");

  // Call the Begin method to connect to an AP
  if (wifi.Begin(MY_SSID, MY_PWD)) {
    Serial.printf("Connected\n");  }
  else {
    Serial.printf("Connection failed\n");
  }
}

void loop() {
  // GetRSSI returns the signal strength
  Serial.printf("RSSI = %d\n", wifi.GetRSSI());

  // GetStatus returns the connection status
  Serial.printf("Status = %d\n", wifi.GetStatus());

  // GetIPAddress returns the wifi IP address
  Serial.printf("IPaddress = %s\n", wifi.GetIPAddress().c_str());

  // GetDNS returns the DNS server IP address
  Serial.printf("DNS = %s\n", wifi.GetDNS().c_str());

  delay(10000);
}
