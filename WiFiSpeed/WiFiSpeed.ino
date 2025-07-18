//----------------------------------------------------------------------
// WiFi speed test
// ---------------
// This creates a test HTTP server.
// Any HTTP connection sends back a large file of random data and the
// transfer time is measured to determine the speed.
// The sketch also monitors the CPU usage. This isn't essential but it's
// interesting info.
//----------------------------------------------------------------------
#include <WiFi.h>
#include "SysCPUMon.h"

// Length of the data sent to the client
#define DATALEN 8388608
// Number of bytes sent in one call to client.write()
// This needs to divide DATALEN e.g. set it to a power of 2
#define BUFLEN  4096

//----------------------------------------------------------------------
// ConnectWiFi
// -----------
// Connect the wifi
//----------------------------------------------------------------------
bool ConnectWiFi(const char* ssid, const char* password, bool powersave) {
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
  return true;
}

//----------------------------------------------------------------------
// WiFiTask
// --------
// The WiFi server runs in this task
//----------------------------------------------------------------------
void WiFiTask(void* Parameter)
{
  Serial.println("Started WiFi task");

  WiFiServer server(80);
  server.begin();

  // Sit in a loop accepting connections
  while (true) {
    // Wait for a client to connect. NB this does not block.
    WiFiClient client = server.accept();
    if (!client) {
      delay(100);
      continue;
    }
    Serial.print("Accepted WiFi connection from ");
    Serial.println(client.remoteIP());

    // Ignore the request details
    uint8_t* buf = (uint8_t*) malloc(BUFLEN);
    while (client.available()) {
      int n = client.read(buf, BUFLEN-1);
      // We could print the header if we were interested
      // buf[n] = '\0';
      // Serial.printf("%s", buf);
    }

    // Write the HTTP header
    client.write("HTTP/1.1 200 OK\r\n");
    client.write("Content-Type: application/octet-stream\r\n");
    client.write("Content-Length: 8388608\r\n");
    client.write("\r\n");

    // Write the data to the client
    for (int i = 0; i < BUFLEN; i++)
      buf[i] = i;

    int t = millis();
    for (int i = 0; i < DATALEN/BUFLEN; i++)
      client.write(buf, BUFLEN);
    client.stop();
    t = millis() - t;
    Serial.printf("WiFi time = %d ms, speed %d KB/s\n", t, DATALEN/t);
  }
}

//----------------------------------------------------------------------
// setup
//----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  // Connect the WiFi
  TaskHandle_t hWiFi;
  ConnectWiFi("MySSID", "MyPassword", false);
  xTaskCreate(WiFiTask, "WiFiTask", 4000, NULL, 1, &hWiFi);

  // Start the CPU monitoring
  StartSystemCPUMon();

  // Finally suspend this thread since all the work is done by other tasks
  vTaskSuspend(NULL);
}

//----------------------------------------------------------------------
// loop
// ----
// This will never be called because setup() never exits
//----------------------------------------------------------------------
void loop() {
}