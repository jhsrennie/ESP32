//----------------------------------------------------------------------
// This code demonstrates access to WiFi driver counters
// on ESP32 / ESP32-S3 in Arduino Framework.
//----------------------------------------------------------------------
#include <WiFi.h>

#define MYSSID "foo"
#define MYPASS "bar"

// Counters incremented by the WiFi driver.
// I did not find any place in Espressif's code
// where these counters are cleared, so it is up to
// the user to reset them if needed.
//
// The counters are cleared on startup, but are not
// reset when the interface goes up or down.
//
typedef struct {
  // STA RX stats
  uint32_t sta_total_rx;  // Total number of WiFi frames received by the STA
  uint32_t sta_data;    // Total number of WiFi data frames received by the STA
  uint16_t sta_ctl;     // Control frames
  uint16_t sta_mgmt;    // Management frames
  uint16_t sta_bcn_probe;   // Beacon/Probe frames

  // It is not clear why the counters below are 8-bit.
  // Perhaps Espressif saves memory, assuming these
  // events are relatively rare.
  //
  // In any case, all counters in this structure
  // may (and will) overflow and wrap around.
  uint8_t  sta_assoc;     // Number of ASSOC requests?
  uint8_t  sta_auth;    // Number of successful authentication attempts?
  uint8_t  sta_deauth;    // Number of deauth frames?
  uint8_t  sta_action;    // Number of action frames?
  uint16_t sta_amsdu;     // AMSDU frames

  // AP RX stats
  uint32_t ap_total_rx;   // Same as above, but for the AP interface
  uint32_t ap_data;
  uint16_t ap_ctl;
  uint16_t ap_mgmt;
  uint16_t ap_bcn_probe;
  uint8_t  ap_assoc;
  uint8_t  ap_auth;
  uint8_t  ap_deauth;
  uint8_t  ap_action;
  uint16_t ap_amsdu;

  // TX stats
  uint32_t ap_total_tx;   // AP: total number of transmitted frames
  uint32_t sta_total_tx;  // STA: total number of transmitted frames

  // Power Save Queue? Not sure about this block.
  // Structure field names were taken from Espressif code.
  //
  uint16_t psq_tx;      // Frames queued?
  uint16_t psq_mc;      // Multicast frames queued?
  uint16_t psq_uc;      // Unicast frames queued?
  uint16_t reorder;     // AMPDU reorder events? <-- incremented on some errors
  uint16_t oos;       // Out-of-sequence events? <-- incremented on some errors
  uint16_t ps_uc1;      // Unknown
  uint16_t tx_amsdu;    // TX AMSDU

} hmac_cnt_t;

// Declare the counters variable.
// The linker does the rest.
//
#ifdef __cplusplus
extern "C" {
#endif

// Global counters structure
extern hmac_cnt_t g_hmac_cnt;

// Function to get the signal to noise
int rom_check_noise_floor(void);

#ifdef __cplusplus
}
#endif

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
  delay(1000);
  Serial.println("Starting WiFiCounters ...");
  ConnectWiFi(MYSSID, MYPASS, false);
}

//----------------------------------------------------------------------
// loop
// ----
//----------------------------------------------------------------------
void loop() {
  Serial.printf(
    "STA total frames received=%lu, data frames=%lu, ctrl=%u, mgmt=%u\r\n"
    "STA total frames sent %lu\r\n",
    g_hmac_cnt.sta_total_rx,
    g_hmac_cnt.sta_data,
    g_hmac_cnt.sta_ctl,
    g_hmac_cnt.sta_mgmt,
    g_hmac_cnt.sta_total_tx
  );

  int nf = rom_check_noise_floor()/4;
  Serial.printf("RSSI = %d, noise = %d, SNR = %d\n", WiFi.RSSI(), nf, WiFi.RSSI() - nf);

  delay(5000);
}