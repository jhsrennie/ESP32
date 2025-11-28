// *********************************************************************
// CRHSWiFi
// --------
// Class to manage WiFi connections using the ESP-IDF
// functions
// *********************************************************************

#include <WiFi.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <esp_event.h>
#include "RHSWiFi.h"

// *********************************************************************
// Constants
// *********************************************************************

#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define WIFI_MAXIMUM_RETRY  5

// Event group bits to monitor the connection status
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// *********************************************************************
// event_handler
// -------------
// Function to handle events.
// This is not a class function. It is passed the address of the object
// in the pRHSWiFi argument.
// *********************************************************************

static void event_handler(void* Arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  // The WiFi object is passed as the first argument
  CRHSWiFi* pRHSWiFi = (CRHSWiFi*) Arg;

  // Number of connect retries
  static int s_retry_num = 0;

  // WiFi events
  if (event_base == WIFI_EVENT) {
    // WiFi starting
    if (event_id == WIFI_EVENT_STA_START) {
      esp_wifi_connect();
      pRHSWiFi->SetStatus(0);
    }

    // WiFi connected and DHCP starting
    // Argument structure for WIFI_EVENT_STA_CONNECTED event
    // typedef struct {
    //     uint8_t ssid[32];         /**< SSID of connected AP */
    //     uint8_t ssid_len;         /**< SSID length of connected AP */
    //     uint8_t bssid[6];         /**< BSSID of connected AP*/
    //     uint8_t channel;          /**< channel of connected AP*/
    //     wifi_auth_mode_t authmode;/**< authentication mode used by AP*/
    //     uint16_t aid;             /**< authentication id assigned by the connected AP */
    // } wifi_event_sta_connected_t;
    else if (event_id == WIFI_EVENT_STA_CONNECTED) {
    }

    // WiFi didn't connect so we need to try again
    // Argument structure for WIFI_EVENT_STA_DISCONNECTED event
    // typedef struct {
    //     uint8_t ssid[32];         /**< SSID of disconnected AP */
    //     uint8_t ssid_len;         /**< SSID length of disconnected AP */
    //     uint8_t bssid[6];         /**< BSSID of disconnected AP */
    //     uint8_t reason;           /**< reason of disconnection */
    //     int8_t  rssi;             /**< rssi of disconnection */
    // } wifi_event_sta_disconnected_t;
    else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
      // If we have not reached the maximum retries we can try again
      if (s_retry_num < WIFI_MAXIMUM_RETRY) {
        esp_wifi_connect();
        s_retry_num++;
      }
      // If we hit the retries the connection hs failed
      else {
        pRHSWiFi->SetFailedBit();
      }
      pRHSWiFi->SetStatus(2);
    }
  }

  // IP events
  else if (event_base == IP_EVENT) {
    // Connected and got IP address
    // Event structure for IP_EVENT_STA_GOT_IP, IP_EVENT_ETH_GOT_IP events
    // typedef struct {
    //     esp_ip4_addr_t ip;      /**< Interface IPV4 address */
    //     esp_ip4_addr_t netmask; /**< Interface IPV4 netmask */
    //     esp_ip4_addr_t gw;      /**< Interface IPV4 gateway address */
    // } esp_netif_ip_info_t;
    if (event_id == IP_EVENT_STA_GOT_IP) {
      ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
      pRHSWiFi->SetIPAddress(event->ip_info.ip.addr, event->ip_info.netmask.addr, event->ip_info.gw.addr);
      pRHSWiFi->SetConnectedBit();
      pRHSWiFi->SetStatus(1);
      s_retry_num = 0;
    }
  }
}

// *********************************************************************
// Constructor
// -----------
// *********************************************************************

CRHSWiFi::CRHSWiFi() {
  m_ip_address = m_subnet_mask = m_def_gate = 0;
  m_status = 0;
  m_netif = NULL;
}

// *********************************************************************
// Begin
// -----
// *********************************************************************

bool CRHSWiFi::Begin(const char* SSID, const char* PWD)
{
  m_wifi_event_group = xEventGroupCreate();

  esp_netif_init();
  // Create the default event loop so we can use it for WiFi notifications
  esp_event_loop_create_default();

  // Create the WiFi station
  m_netif = esp_netif_create_default_wifi_sta();

  // Initialise the WiFi using the default settings
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);

  // Register an event handler for all WiFi events
  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, this, &instance_any_id);

  // Register an event handle for the "got IP address" event
  esp_event_handler_instance_t instance_got_ip;
  esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &event_handler, this, &instance_got_ip);

  // Initialise the config
  wifi_config_t wifi_config;
  memset(&wifi_config, 0, sizeof(wifi_config));
  strcpy((char*)wifi_config.sta.ssid, SSID);
  strcpy((char*)wifi_config.sta.password, PWD);
  wifi_config.sta.threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD;
  wifi_config.sta.sae_pwe_h2e = ESP_WIFI_SAE_MODE;

  // Set the configuration and start the WiFi
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
  esp_wifi_start();

  /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
  * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
  EventBits_t bits = xEventGroupWaitBits(
    m_wifi_event_group,
    WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
    pdFALSE,
    pdFALSE,
    portMAX_DELAY);

  // Check the bits to see what happened
  if (bits & WIFI_FAIL_BIT)
    return false;

  // Return indicating success
  return true;
}

// *********************************************************************
// GetRSSI
// -------
// *********************************************************************

int CRHSWiFi::GetRSSI(void) {
  int rssi;

  return esp_wifi_sta_get_rssi(&rssi) == ESP_OK ? rssi : 0;
}

// *********************************************************************
// Getters and Setters
// -------------------
// *********************************************************************

void CRHSWiFi::SetConnectedBit(void) {
  xEventGroupSetBits(m_wifi_event_group, WIFI_CONNECTED_BIT);
}

void CRHSWiFi::SetFailedBit(void) {
  xEventGroupSetBits(m_wifi_event_group, WIFI_FAIL_BIT);
}

void CRHSWiFi::SetIPAddress(uint32_t IPAddress, uint32_t Subnet, uint32_t DefGate) {
  m_ip_address = IPAddress;
  m_subnet_mask = Subnet;
  m_def_gate = DefGate;
}

String CRHSWiFi::GetIPAddress(void) {
  String ip = String(m_ip_address & 0xFF) + "."
            + String((m_ip_address >> 8) & 0xFF) + "."
            + String((m_ip_address >> 16) & 0xFF) + "."
            + String(m_ip_address >> 24);

  return ip;
}

String CRHSWiFi::GetDNS(void) {
  String ip = "0.0.0.0";

  if (!m_netif)
    return ip;

  esp_netif_dns_info_t dns;
  if (esp_netif_get_dns_info((esp_netif_t*) m_netif, ESP_NETIF_DNS_MAIN, &dns) == ESP_OK) {
    uint32_t dnsaddr = dns.ip.u_addr.ip4.addr;
    ip = String(dnsaddr & 0xFF) + "."
       + String((dnsaddr >> 8) & 0xFF) + "."
       + String((dnsaddr >> 16) & 0xFF) + "."
       + String(dnsaddr >> 24);
  }

  return ip;
}

void CRHSWiFi::SetStatus(int Status) {
  m_status = Status;
}

int CRHSWiFi::GetStatus(void) {
  return m_status;
}
