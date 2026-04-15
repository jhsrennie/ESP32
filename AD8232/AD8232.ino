//----------------------------------------------------------------------
// AD8232 Heart Monitor
// ====================
//----------------------------------------------------------------------
#include <WiFi.h>
#include "Server.h"
#include "Data.h"

// WiFi credentials
#define WIFI_SSID "RatHaus"
#define WIFI_PASS "rattusrattus"

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
//----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting Heart Monitor");

  // Start the data collection task
  if (DataStart()) {
    Serial.println("Data collection started");
  }
  else {
    Serial.println("Error starting data collection");
    vTaskSuspend(NULL);
  }

  // Connect the WiFi
  ConnectWiFi(WIFI_SSID, WIFI_PASS, false);

  // Setup the web server
  ServerSetup();

  // Done
  Serial.println("Started Heart Monitor");
  Serial.print("Connect to http://");
  Serial.print(WiFi.localIP());
  Serial.println("/ to view the data");
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