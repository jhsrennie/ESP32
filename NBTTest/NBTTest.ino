//----------------------------------------------------------------------
// NBTTest
// =======
// Code to demonstrate how to listen for and respond to NBT name
// resolution requests.
//----------------------------------------------------------------------
#include <WiFi.h>
#include "NBTListener.h"

// This is the name we will respond to
#define NBTNAME "foo"

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
  Serial.println("Starting NBTTest ...");

  // Connect the wi-fi
  ConnectWiFi(MYSSID, MYPASS, false);

  // Create the name resolution task
  Serial.println("Starting Netbios listener ...");
  int e = StartNBTListener(NBTNAME, WiFi.localIP().toString().c_str());
  if (e != NBT_ERR_OK)
    Serial.printf("Failed to start the NBT listener: error %d", e);

  // All done
  Serial.println("NBTTest started.");
}

//----------------------------------------------------------------------
// loop
// ----
//----------------------------------------------------------------------
void loop() {
  // Now do something useful and the NBT listener will run in the
  // background as a separatre task.
  delay(1000);
}