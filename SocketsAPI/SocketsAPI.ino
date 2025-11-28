#include <lwip/sockets.h>
#include "WiFi.h"

// WiFi credentials
String SSID = "MySSID";
String PWD  = "MyPassword";

void ConnectWiFi() {
  WiFi.begin(SSID, PWD, 6);
  int loopcnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf("Connecting: time %d, WiFi status = %d, signal = %d\n", loopcnt++, WiFi.status(), WiFi.RSSI());
    delay(1000);
  }
  Serial.printf("Connected: %s\n", WiFi.localIP().toString().c_str());
}

void setup() {
  Serial.begin(115200);
  ConnectWiFi();
}

void loop() {
  // Create, bind and listen on port 23
  int sock_srv = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  
  struct sockaddr addr_in;
  memset(&addr_in, 0, sizeof(addr_in));
  struct sockaddr_in* p_addr_in = (struct sockaddr_in*) &addr_in;
  p_addr_in->sin_family = PF_INET;
  p_addr_in->sin_port = htons(23);
  p_addr_in->sin_addr.s_addr = INADDR_ANY;
  bind(sock_srv, &addr_in, sizeof(addr_in));

  listen(sock_srv, 1);
 
  // Loop to handle connections
  while (true) {
    // Wait for a connection
    sockaddr addr_client;
    memset(&addr_client, 0, sizeof(addr_client));
    int sock_client = accept(sock_srv, &addr_client, NULL);
    Serial.println("Accepted connection");

    // Just end a message then close the socket
    String s = "Welcome and goodbye!\r\n";
    send(sock_client, s.c_str(), s.length(), 0);
    closesocket(sock_client);
  }   
}