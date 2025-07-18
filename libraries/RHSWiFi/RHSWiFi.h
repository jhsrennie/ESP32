#ifndef _INC_CRHSWiFi
#define  _INC_CRHSWiFi

// *****************************************************
// CRHSWiFi
// --------
// Class to manage WiFi connections using the ESP-IDF
// functions
// *****************************************************

class CRHSWiFi {
  public:
    CRHSWiFi();

    bool Begin(const char* SSID, const char* PWD);

    int GetRSSI(void);

    void SetConnectedBit(void);
    void SetFailedBit(void);
    void SetIPAddress(uint32_t IPAddress, uint32_t Subnet = 0, uint32_t DefGate = 0);
    String GetIPAddress(void);
    String GetDNS(void);
    void SetStatus(int Status);
    int GetStatus(void);
  
  private:
    // This is the esp_netif_t* for the interface created by
    // esp_netif_create_default_wifi_sta()
    void* m_netif;

    EventGroupHandle_t m_wifi_event_group;
    uint32_t m_ip_address, m_subnet_mask, m_def_gate;
    int m_status; // 0 = not connected, 1 = connected, 2 = failed
};
#endif // _INC_CRHSWiFi