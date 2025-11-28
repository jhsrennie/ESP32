//----------------------------------------------------------------------
// W5500 ethernet module test
// This creates a test HTTP server on both the W5500 and the WiFi
// Any HTTP connection sends back a large file of random data and the
// transfer time is measured to determine the speed.
//----------------------------------------------------------------------
#include <WiFi.h>
#include <SPI.h>
#include <Ethernet.h>

// The MAC address of the w5500 needs to be supplied
// You can make up any random address
byte W5500MAC[] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };

// Length of the data sent to the client
#define DATALEN 4194304
// Number of bytes sent in one call to client.write()
#define BUFLEN  512

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
// ConnectW5500
// ------------
// Connect the W5500 shield
//----------------------------------------------------------------------
bool ConnectW5500() {
  // We need to specify the SS pin being used
  Ethernet.init(SS);
  // initialize the Ethernet shield using DHCP:
  if (Ethernet.begin(W5500MAC) == 0) {
    Serial.println("Initialisation failed");
    // check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
      Serial.println("Ethernet shield was not found");
    // check for Ethernet cable
    if (Ethernet.linkStatus() == LinkOFF)
      Serial.println("Ethernet cable is not connected.");
    // Give up at this point
    return false;
  }

  // Print the details
  Serial.printf("Ethernet chip ID = %d\n", Ethernet.hardwareStatus());
  Serial.print("Ethernet initialised, IP address = ");
  Serial.println(Ethernet.localIP());
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
      // buf[n] = '\0';
      // Serial.printf("%s", buf);
    }

    // Write the HTTP header
    client.write("HTTP/1.1 200 OK\r\n");
    client.write("Content-Type: application/octet-stream\r\n");
    client.write("Content-Length: 4194304\r\n");
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
// W5500Task
// ---------
// The W5500 server runs in this task
//----------------------------------------------------------------------
void W5500Task(void* Parameter)
{
  Serial.println("Started W5500 task");

  EthernetServer server(80);
  server.begin();

  // Sit in a loop accepting connections
  while (true) {
    // Wait for a client to connect. NB this does not block.
    EthernetClient client = server.accept();
    if (!client) {
      delay(100);
      continue;
    }
    Serial.print("Accepted Ethernet connection from ");
    Serial.println(client.remoteIP());

    // Ignore the request details
    uint8_t* buf = (uint8_t*) malloc(BUFLEN);
    while (client.available()) {
      int n = client.read(buf, BUFLEN-1);
    }

    // Write the HTTP header
    client.write("HTTP/1.1 200 OK\r\n");
    client.write("Content-Type: application/octet-stream\r\n");
    client.write("Content-Length: 4194304\r\n");
    client.write("\r\n");

    // Write the data to the client
    for (int i = 0; i < BUFLEN; i++)
      buf[i] = i;

    int t = millis();
    for (int i = 0; i < DATALEN/BUFLEN; i++)
      client.write(buf, BUFLEN);
    client.stop();
    t = millis() - t;
    Serial.printf("W5500 time = %d ms, speed %d KB/s\n", t, DATALEN/t);
  }
}

//----------------------------------------------------------------------
// CPUUsageTask
// ------------
// The task monitors the CPU usage
//----------------------------------------------------------------------
void CPUUsageTask(void* Parameter)
{
  int num_cpus;
  unsigned long runtime;
  unsigned long last_runtime[2] = {0};
  TaskHandle_t htask[2];

  // Print the task stats summary
  char* buf = (char*) malloc(1000);
  Serial.println("Task list:");
  vTaskGetRunTimeStats(buf);
  Serial.print(buf);

  // We need the IDLE task so we can follow its CPU usage
  // On single core ESP32s this is just called "IDLE"
  htask[0] = xTaskGetHandle("IDLE");
  if (htask[0]) {
    num_cpus = 1;
  }
  // On dual core ESP32s there are two tasks "IDLE0" and "IDLE1"
  else {
    htask[0] = xTaskGetHandle("IDLE0");
    htask[1] = xTaskGetHandle("IDLE1");
    num_cpus = 2;
    // If we cannot find the idle task give up
    if (!htask[0] || !htask[1]) {
      Serial.println("CPUUsage: cannot find IDLE task");
      vTaskSuspend(NULL);
    }
  }

  // Loop checking the CPU usage by the IDLE task(s)
  while (true) {
    TaskStatus_t xTaskDetails;

    // Get the runtime for the first idle task
    vTaskGetInfo(htask[0], &xTaskDetails, pdTRUE, eInvalid);
    runtime = xTaskDetails.ulRunTimeCounter - last_runtime[0];
    last_runtime[0] = xTaskDetails.ulRunTimeCounter;
    // If there is a second CPU get its runtime as well
    if (num_cpus == 2) {
      vTaskGetInfo(htask[1], &xTaskDetails, pdTRUE, eInvalid);
      runtime += xTaskDetails.ulRunTimeCounter - last_runtime[1];
      last_runtime[1] = xTaskDetails.ulRunTimeCounter;
    }
    // Calculate the run time used
    float pc_idle = runtime/10000.0;
    Serial.printf("IDLE: %.1f\n", pc_idle);
    // Pause for a second
    delay(1000);
  }
}

//----------------------------------------------------------------------
// setup
//----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  // Connect the WiFi
  TaskHandle_t hWiFi;
  ConnectWiFi("myssid", "mypassword", false);
  xTaskCreate(WiFiTask, "WiFiTask", 4000, NULL, 1, &hWiFi);

  // Connect the W5500
  TaskHandle_t hW5500;
  ConnectW5500();
  xTaskCreate(W5500Task, "W5500Task", 4000, NULL, 1, &hW5500);

  // Create the task to monitor CPU
  TaskHandle_t hCPU;
  xTaskCreate(CPUUsageTask, "CPUUsageTask", 3000, NULL, 1, &hCPU);

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