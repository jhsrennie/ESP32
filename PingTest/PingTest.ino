//----------------------------------------------------------------------
// Ping
// ====
// Test for the PingSession and LWIPPing classes
//----------------------------------------------------------------------
#include <WiFi.h>
#include "PingSession.h"
#include "LWIPping.h"
#include "ping.h"

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
  Serial.setTimeout(1000*1000); // Long timeout on user input
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting ping test");

  // Start the wi-fi with power saving off
  ConnectWiFi(MYSSID, MYPASS, false);
}

//----------------------------------------------------------------------
// loop
// ----
//----------------------------------------------------------------------
void loop() {
  // Prompt for the host to ping
  Serial.println("\n\nEnter a host to ping:");
  String host = Serial.readStringUntil(10);
  if (host == "")
    return;

  // First try using the ping session class
  PingSession p;
  Serial.printf("Pinging %s using PingSession\n", host.c_str());
  if (p.start(host.c_str(), 4)) {
    Serial.printf("Ping results from %s\n", p.ip_address());
    Serial.printf("Replies = %d, timeouts = %d\n", p.replies(), p.timeouts());
    Serial.printf("Last reply time = %d, min = %d, max = %d\n", p.last_reply_time(), p.min_reply_time(), p.max_reply_time());
    Serial.printf("Average reply time = %d\n", p.av_reply_time());
  }
  else {
    Serial.println(p.last_error_msg());
  }
  p.stop();
  Serial.println("");

  // Now try using the LWIPPing class
  LWIPPing lwp;
  Serial.printf("Pinging %s using the LWIPPing class\n", host.c_str());
  int is_ip = host[0] >= '0' && host[0] <= '9'; // is it a host or IP?
  for (int i = 0; i < 4; i++) {
    int reply_time;
    if (is_ip)
      reply_time = lwp.ping_ip(host.c_str());
    else
      reply_time = lwp.ping_host(host.c_str());
    if (reply_time >= 0)
      Serial.printf("Attempt %d: reply time = %d\n", i, reply_time);
    else
      Serial.printf("Attempt %d: error %d, %s\n", i, lwp.last_error(), lwp.last_error_msg());
  }
  Serial.println("");

  // Now try using the C function
  Serial.printf("Pinging %s using LWIP sockets\n", host.c_str());
  for (int i = 0; i < 4; i++) {
    int reply_time;
    if (is_ip)
      reply_time = ping_ip(host.c_str(), 1500);
    else
      reply_time = ping_host(host.c_str(), 1500);
    if (reply_time >= 0)
      Serial.printf("Attempt %d: reply time = %d\n", i, reply_time);
    else
      Serial.printf("Attempt %d: error %d, %s\n", i, ping_last_error(), ping_last_error_msg());
  }
}