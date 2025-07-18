//----------------------------------------------------------------------
// ESP-NOW test program
// This reads strings typed in at the serial monitor and sends them to
// all MAC addresses in the Peers array.
//----------------------------------------------------------------------
#include <WiFi.h>
#include "esp_now.h"

// The original ESP32 has the LED on pin 2 and HIGH turns the LED on
// while LOW turns it off.
#if CONFIG_IDF_TARGET_ESP32
#define LED_GPIO 2
#define LED_ON  HIGH
#define LED_OFF LOW

// The ESP32 C3 has the LED on pin 8 and HIGH turns the LED off while
// LOW turns it on.
#elif CONFIG_IDF_TARGET_ESP32C3
#define LED_GPIO 8
#define LED_ON  LOW
#define LED_OFF HIGH
#endif

// This is the array of all peer MAC addresses
// Our MAC address must be in this list
// I filled this in with the MAC addresses of my ESP32 and ESP32-C3 for
// testing. You need to replace this with the addresses of your boards.
#define NUM_PEERS 2
uint8_t Peers[NUM_PEERS][6] = {
  {0xa0, 0xdd, 0x6c, 0x04, 0x57, 0x44}, // ESP32
  {0xf0, 0xf5, 0xbd, 0xfc, 0x23, 0xd0}, // ESP32-C3
};
// Index of the local MAC address in Peers
int localMAC;

// Macro for comparing MAC addresses
#define MAC_EQUAL(a, b) (a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3] && a[4] == b[4] && a[5] == b[5])

// Macro to copy a MAC address from b to a
#define MAC_COPY(a, b) a[0] = b[0]; a[1] = b[1]; a[2] = b[2]; a[3] = b[3]; a[4] = b[4]; a[5] = b[5];

// Macro to provide the MAC address arguments for printf
#define MAC_PRINTF(a) a[0], a[1], a[2], a[3], a[4], a[5]

//----------------------------------------------------------------------
// This function connects to the wireless network
//----------------------------------------------------------------------
void connectWiFi() {
  WiFi.begin();
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(WIFI_PS_NONE);

  // Get the MAC address
  uint8_t mac[6] = {0};
  WiFi.macAddress(mac);
  Serial.printf("MAC address = %02x-%02x-%02x-%02x-%02x-%02x\n", MAC_PRINTF(mac));

  // Find the local MAC address in the list of peers
  for (localMAC = 0; localMAC < NUM_PEERS; localMAC++)
    if (MAC_EQUAL(mac, Peers[localMAC]))
      break;
  // If we didn't find our address that's a fatal error so hlt now
  if (localMAC == NUM_PEERS) {
    Serial.println("Error: MAC address is not in list");
    vTaskSuspend (NULL);
  }
  Serial.printf("Local MAC index = %d\n", localMAC);
}

//----------------------------------------------------------------------
// Function called when data is received
//----------------------------------------------------------------------
void onRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len) {
  String s(data, data_len);
  Serial.printf("Received: %i dB, %s\n", esp_now_info->rx_ctrl->rssi, s.c_str());
}

//----------------------------------------------------------------------
// Function called when data is sent
//----------------------------------------------------------------------
void onSend(const uint8_t *mac_addr, esp_now_send_status_t status) {
 if (status == ESP_NOW_SEND_SUCCESS)
    Serial.printf("Data sent: %02x-%02x-%02x-%02x-%02x-%02x\n", MAC_PRINTF(mac_addr));
 else
    Serial.printf("Send failed: %02x-%02x-%02x-%02x-%02x-%02x\n", MAC_PRINTF(mac_addr));
}

//----------------------------------------------------------------------
// initNOW
// Initialise ESP-NOW add add all the peers
// Return true if successful or flast if not
//----------------------------------------------------------------------
bool initNOW() {
  // Initialise ESP-NOW
  esp_err_t e = esp_now_init();
  if (e != ESP_OK) {
    Serial.printf("Error: esp_now_init returned %d\n", e);
    return false;
  }
  Serial.println("ESP-NOW initialised");

  // Add the receive callback
  e = esp_now_register_recv_cb(onRecv);
  if (e != ESP_OK) {
    Serial.printf("Error: esp_now_register_recv_cb returned %d\n", e);
    return false;
  }
  Serial.println("Receive callback configured");

  // Add the send callback
  e = esp_now_register_send_cb(onSend);
  if (e != ESP_OK) {
    Serial.printf("Error: esp_now_register_send_cb returned %d\n", e);
    return false;
  }
  Serial.println("Send callback configured");

  // Add the peers
   esp_now_peer_info_t peer = {0};
   for (int i = 0; i < NUM_PEERS; i++) {
    if (i != localMAC) { // Skip our own MAC address
      MAC_COPY(peer.peer_addr, Peers[i])
      e = esp_now_add_peer(&peer);
      if (e != ESP_OK)
        Serial.printf("Error: esp_now_add_peer returned %d for Peers[%d]\n", e, i);
      else
        Serial.printf("Added peer: %02x-%02x-%02x-%02x-%02x-%02x\n", MAC_PRINTF(Peers[i]));
    }
  }

  // Return indicating success
  return true;
}

//----------------------------------------------------------------------
// setup
//----------------------------------------------------------------------
void setup() {
  // Connect the serial monitor and wait for it to initialise
  Serial.begin(115200);
  delay(1000);

  // Connect the WiFi
  connectWiFi();

  // Add all the peers
  initNOW();

  // Flash the LED to indicate it is connected
  pinMode(LED_GPIO, OUTPUT);
  digitalWrite(LED_GPIO, LED_ON);
  delay(1000);
  digitalWrite(LED_GPIO, LED_OFF);
}

//----------------------------------------------------------------------
// loop
//----------------------------------------------------------------------
void loop() {
  // read a string from the serial monitor
  if (Serial.available()) {
    String s = Serial.readStringUntil(10);

    // Send the string
    Serial.printf("Sending \"%s\"\n", s.c_str());
    for (int i = 0; i < NUM_PEERS; i++) {
      if (i != localMAC) {
        Serial.printf("Sending to: %02x-%02x-%02x-%02x-%02x-%02x\n", MAC_PRINTF(Peers[i]));
        esp_err_t e = esp_now_send(Peers[i], (uint8_t*) s.c_str(), s.length());
        if (e != ESP_OK)
          Serial.printf("Error: esp_now_send returned %d\n", e);
      }
    }
  }
  delay(100);
}