//----------------------------------------------------------------------
// time.cpp
// ========
// Time functions
//----------------------------------------------------------------------
#include <WiFi.h>
#include <esp_netif_sntp.h>
#include <time.h>
#include "clock.h"

// NTP server details
#define TIMEZONE  "GMT0BST,M3.5.0/1,M10.5.0"
#define NTPSERVER "pool.ntp.org"
#define NTP_RETRIES 3
#define NTP_FAILS   3

//----------------------------------------------------------------------
// UpdateTime
//-----------
//----------------------------------------------------------------------
void UpdateTime(void* Unused) {
#define MAX_RETRIES 3
  int time_retries;

  // Initialise the SNTP system
  esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(NTPSERVER);

  // Loop continuously updating the time once an hour
  for (;;) {
    // Enable the wi-fi
    DisplayStatus("Connecting to WiFi ...");
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(WIFI_PS_NONE);

    const char* wifi_ssid = GetSSID();
    const char* wifi_pass = GetPassword();
    WiFi.begin(wifi_ssid, wifi_pass);
    int loopcnt = 0;
    Serial.printf("Connecting to WiFi %s ...", wifi_ssid);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.printf("Connecting: time %d, WiFi status = %d, signal = %d\n", loopcnt++, WiFi.status(), WiFi.RSSI());
      delay(1000);
    }
    Serial.println(WiFi.localIP());

    // Get the time
    Serial.println("Getting the time ...");
    DisplayStatus("Getting the time ...");

    for (time_retries = 0; time_retries < MAX_RETRIES; time_retries++) {
      Serial.printf("Attempt %d ... ", time_retries);

      // Call init to start the sync
      esp_netif_sntp_init(&config);
      configTzTime(TIMEZONE, NTPSERVER);

      // And wait for the sync to complete
      esp_err_t e = esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000));
      // Deinitialise after the sync has completed
      esp_netif_sntp_deinit();

      // Check if the sync succeeded
      if (e == ESP_OK) {
        Serial.println("succeeded");
        struct timeval tv_now;
        gettimeofday(&tv_now, NULL);
        time_t tt = (time_t) tv_now.tv_sec;
        struct tm* tm_now = localtime(&tt);
        DisplayStatus("Synced with %s at %02d:%02d:%02d", NTPSERVER, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
        // And exit the loop since we succeeded
        break;
      }
      else {
        Serial.printf("failed: %s\n", esp_err_to_name(e));
        DisplayStatus("Time sync failed");
      }
    }

    // Disable the wi-fi to save power
    WiFi.disconnect();

    // And wait an hour
    delay(3600*1000);
  }
}