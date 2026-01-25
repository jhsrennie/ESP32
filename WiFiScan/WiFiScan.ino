//----------------------------------------------------------------------
// WiFiScan
// ========
// Scan the local wi-fi networks for MAC addresses
//----------------------------------------------------------------------
#include <esp_wifi.h>

// List of MAC addresses we have sniffed
#define MAX_MACADDR 100
uint8_t macaddrs[MAX_MACADDR][6];
int num_macaddrs = 0;

// Macro for comparing MAC addresses
#define MAC_EQUAL(a, b) (a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3] && a[4] == b[4] && a[5] == b[5])

// Macro to copy a MAC address from b to a
#define MAC_COPY(a, b) a[0] = b[0]; a[1] = b[1]; a[2] = b[2]; a[3] = b[3]; a[4] = b[4]; a[5] = b[5];

// Macro to provide the MAC address arguments for printf
#define MAC_PRINTF(a) a[0], a[1], a[2], a[3], a[4], a[5]

// The WifiMgmtHdr is the first part of the packet data
// This is not in the standard headers so we have to define it here
typedef struct {
  int16_t fctl;
  int16_t duration;
  uint8_t da[6];    // destination MAC address
  uint8_t sa[6];    // source MAC address
  uint8_t bssid[6]; // router MAC address
  int16_t seqctl;
  unsigned char payload[];
} __attribute__((packed)) WifiMgmtHdr;

// max channel is US = 11, EU = 13, Japan = 14
#define MAX_CHANNEL 13
int cur_channel = 1;

//----------------------------------------------------------------------
// sniffer
// -------
// Callback function called when a packet is sniffed
// buf points to the packet data
//
// The packet data has this layout:
// typedef struct {
//     wifi_pkt_rx_ctrl_t rx_ctrl;
//     uint8_t payload[0]; //Length of payload is given by rx_ctrl.sig_len.
// } wifi_promiscuous_pkt_t;
//
// where the wifi_pkt_rx_ctrl_t has the structure:
// typedef struct {
//   signed rssi: 8;                // Received Signal Strength Indicator(RSSI) of packet. unit: dBm
//   unsigned rate: 5;              // PHY rate encoding of the packet. Only valid for non HT(11bg) packet
//   unsigned : 1;                  // reserved
//   unsigned sig_mode: 2;          // Protocol of the received packet, 0: non HT(11bg) packet; 1: HT(11n) packet; 3: VHT(11ac) packet
//   unsigned : 16;                 // reserved
//   unsigned mcs: 7;               // Modulation Coding Scheme. If is HT(11n) packet, shows the modulation, range from 0 to 76(MSC0 ~ MCS76)
//   unsigned cwb: 1;               // Channel Bandwidth of the packet. 0: 20MHz; 1: 40MHz
//   unsigned : 16;                 // reserved
//   unsigned smoothing: 1;         // Set to 1 indicates that channel estimate smoothing is recommended.
//                                  // Set to 0 indicates that only per-carrierindependent (unsmoothed) channel estimate is recommended.
//   unsigned not_sounding: 1;      // Set to 0 indicates that PPDU is a sounding PPDU. Set to 1indicates that the PPDU is not a sounding PPDU.
//                                  // sounding PPDU is used for channel estimation by the request receiver
//   unsigned : 1;                  // reserved
//   unsigned aggregation: 1;       // Aggregation. 0: MPDU packet; 1: AMPDU packet
//   unsigned stbc: 2;              // Space Time Block Code(STBC). 0: non STBC packet; 1: STBC packet
//   unsigned fec_coding: 1;        // Forward Error Correction(FEC). Flag is set for 11n packets which are LDPC
//   unsigned sgi: 1;               // Short Guide Interval(SGI). 0: Long GI; 1: Short GI
// #if CONFIG_IDF_TARGET_ESP32
//   signed noise_floor: 8;         // noise floor of Radio Frequency Module(RF). unit: dBm
// #elif CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C2
//   unsigned : 8;                  // reserved
// #endif
//   unsigned ampdu_cnt: 8;         // the number of subframes aggregated in AMPDU
//   unsigned channel: 4;           // primary channel on which this packet is received
//   unsigned secondary_channel: 4; // secondary channel on which this packet is received. 0: none; 1: above; 2: below
//   unsigned : 8;                  // reserved
//   unsigned timestamp: 32;        // timestamp. The local time when this packet is received. It is precise only if modem sleep or light sleep is not enabled. unit: microsecond
//   unsigned : 32;                 // reserved
// #if CONFIG_IDF_TARGET_ESP32S2
//   unsigned : 32;                 // reserved
// #elif CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C2
//   signed noise_floor: 8;         // noise floor of Radio Frequency Module(RF). unit: dBm
//   unsigned : 24;                 // reserved
//   unsigned : 32;                 // reserved
// #endif
//   unsigned : 31;                 // reserved
//   unsigned ant: 1;               // antenna number from which this packet is received. 0: WiFi antenna 0; 1: WiFi antenna 1
// #if CONFIG_IDF_TARGET_ESP32S2
//   signed noise_floor: 8;         // noise floor of Radio Frequency Module(RF). unit: dBm
//   unsigned : 24;                 // reserved
// #elif CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32C2
//   unsigned : 32;                 // reserved
//   unsigned : 32;                 // reserved
//   unsigned : 32;                 // reserved
// #endif
//   unsigned sig_len: 12;          // length of packet including Frame Check Sequence(FCS)
//   unsigned : 12;                 // reserved
//   unsigned rx_state: 8;          // state of the packet. 0: no error; others: error numbers which are not public
// } wifi_pkt_rx_ctrl_t;
//----------------------------------------------------------------------
void sniffer(void* buf, wifi_promiscuous_pkt_type_t type) {
  // The buffer contains a wifi_promiscuous_pkt_t i.e. a wifi_pkt_rx_ctrl_t
  // followed by the packet data
  wifi_promiscuous_pkt_t *p = (wifi_promiscuous_pkt_t*)buf;
  // The length of the packet data is in the wifi_pkt_rx_ctrl_t sig_len field
  int len = p->rx_ctrl.sig_len;
  // The WifiMgmtHdr is the first part of the packet data
  WifiMgmtHdr *wh = (WifiMgmtHdr*)p->payload;
  // Once we've subtracted off the WifiMgmtHdr what's left is the packet data
  len -= sizeof(WifiMgmtHdr);

  // Check the source MAC address to see if it's in our list
  int i;
  for (i = 0; i < num_macaddrs; i++)
    if (MAC_EQUAL(macaddrs[i], wh->sa))
      break;
  // Did we get a new address?
  if (i >= num_macaddrs) {
    if (num_macaddrs < MAX_MACADDR) {
      MAC_COPY(macaddrs[num_macaddrs], wh->sa);
      Serial.printf("%4d  %4d  %02x-%02x-%02x-%02x-%02x-%02x\n", cur_channel, p->rx_ctrl.rssi, MAC_PRINTF(macaddrs[num_macaddrs]));
      num_macaddrs++;
    }
  }
}

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Started WiFiScan");

  // We need to create the default event loop even if we don't use it
  esp_event_loop_create_default();

  // Configure the wi-fi in promiscuous mode
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_NULL);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);

  // The filter sets all packet types that are accepted. See:
  // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/wifi-driver/wifi-modes.html#wi-fi-sniffer-mode
  // The default is to accept all packets except WIFI_PKT_MISC
  // Options are:
  // WIFI_PROMIS_FILTER_MASK_ALL
  // WIFI_PROMIS_FILTER_MASK_MGMT
  // WIFI_PROMIS_FILTER_MASK_CTRL
  // WIFI_PROMIS_FILTER_MASK_DATA
  // WIFI_PROMIS_FILTER_MASK_MISC
  // WIFI_PROMIS_FILTER_MASK_DATA_MPDU
  // WIFI_PROMIS_FILTER_MASK_DATA_AMPDU
  // WIFI_PROMIS_FILTER_MASK_FCSFAIL
  wifi_promiscuous_filter_t filter;
  filter.filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT
                     | WIFI_PROMIS_FILTER_MASK_DATA;
  // For now comment this out to use the default
  // esp_wifi_set_promiscuous_filter(&filter);

  // Set the callback function called whenever a packet is accepted
  esp_wifi_set_promiscuous_rx_cb(&sniffer);

  // Print the header
  Serial.println("Chan  RSSI  MAC address");
}

//----------------------------------------------------------------------
// loop
// ----
// The loop scans all wi-fi channels sequentially
//----------------------------------------------------------------------
void loop() {
  // Scan the next channel
  esp_wifi_set_channel(cur_channel, WIFI_SECOND_CHAN_NONE);
  delay(1000);

  // Move to the next channel
  cur_channel++;
  if(cur_channel > MAX_CHANNEL){
      cur_channel = 1;
  }
}