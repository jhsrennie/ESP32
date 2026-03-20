//----------------------------------------------------------------------
// mDNSTest
// ========
// Code to demonstrate how to listen for and respond to mDNS name
// resolution requests.
//----------------------------------------------------------------------
#include <WiFi.h>
#include "mDNSListener.h"

// This is the name we will respond to
#define HOSTNAME "foobar.local"

// Wi-fi credential
#define MYSSID "MYSSID"
#define MYPASS "MYPASS"

//----------------------------------------------------------------------
// ConnectWiFi
// -----------
// Connect the wifi
//----------------------------------------------------------------------
void ConnectWiFi(const char* ssid, const char* password, bool powersave) {
  WiFi.mode(WIFI_STA);
  // Disable power saving if required
  if (!powersave)
    WiFi.setSleep(WIFI_PS_NONE);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi ...");
  int loopcnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf("Connecting: time %d, WiFi status = %d, signal = %d\n", loopcnt++, WiFi.status(), WiFi.RSSI());
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting mDNSTest ...");

  // Connect the wi-fi
  ConnectWiFi(MYSSID, MYPASS, false);

  // Create the name resolution task
  Serial.println("Starting mDNS listener ...");
  int e = StartmDNSListener(HOSTNAME, WiFi.localIP().toString().c_str());
  if (e != MDNS_ERR_OK)
    Serial.printf("Failed to start the mDNS listener: error %d", e);

  // All done
  Serial.println("mDNSTest started.");
}

//----------------------------------------------------------------------
// loop
// ----
//----------------------------------------------------------------------
void loop() {
  // Now do something useful and the mDNS listener will run in the
  // background as a separate task.
  delay(1000);
}