// ---------------------------------------------------------------------
// NTPClient
// ---------
// Demonstrate how to get the time from an NTP server
// This uses only the ESP-IDF SNTP and POSIX time functions
// ---------------------------------------------------------------------
#include <WiFi.h>
#include <esp_netif_sntp.h>
#include <time.h>

// WiFi credentials
#define SSID "MySSID"
#define PWD  "MyPWD"

// NTP server details
#define NTPSERVER "pool.ntp.org"

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
// InitSNTP
// --------
// Initialise the SNTP client and use it to set the time
// ---------------------------------------------------------------------
bool InitSNTP() {
  // Initialise the SNTP client
  esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(NTPSERVER);
  esp_netif_sntp_init(&config);

  // Request the time from the SNTP server
  esp_err_t e = esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000));
  if (e != ESP_OK) {
    Serial.printf("Failed to set system time: %s\n", esp_err_to_name(e));
    return false;
  }

  // Return indicating success
  Serial.printf("Time set from %s\n", NTPSERVER);
  return true;
}

// ---------------------------------------------------------------------
// setup
// ---------------------------------------------------------------------
void setup()
{
  // Initialise the serial port
  Serial.begin(115200);
  
  // Connect to WiFi
  ConnectWiFi();  

  // Initialise the SNTP client
  if (!InitSNTP()) {
    Serial.println("InitSNTP() failed");
  }
}

// ---------------------------------------------------------------------
// loop
// ---------------------------------------------------------------------
void loop()
{
  // Get the time. We have not set a time zone so the time is GMT.
  struct timeval tv_now;
  gettimeofday(&tv_now, NULL);
  time_t tt = (time_t) tv_now.tv_sec;
  struct tm* tm_now = localtime(&tt);

  // Format and print the time
  #define LEN_BUF 128
  char buf[LEN_BUF];
  strftime(buf, LEN_BUF, "%a, %d %b %Y %T %z", tm_now);
  Serial.println(buf);

  delay(10000);
}