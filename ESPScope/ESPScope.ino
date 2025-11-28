// ---------------------------------------------------------------------
// ESPScope
// ---------------------------------------------------------------------

#include <WebServer.h>

// Prototypes
void connectWiFi();
bool setCalibFreq(int Freq);
bool initADC(int Freq);
bool getFrame(uint16_t* Data, uint32_t LenData);

// Use a global variable for the server since there is only one server
WebServer server(80);

// Global arrays to hold the scope data
#define LEN_SCOPEDATA 1000
uint16_t scopeData[LEN_SCOPEDATA];

// Calibration frequency in Hz
int calibFreq = 1000;

// ADC sampling rate in KHz
int adcConvFreq = 200;

// ---------------------------------------------------------------------
// sendFrameSVG
// ---------------------------------------------------------------------

void sendFrameSVG() {
  int x, y;
  String path = "";

  // Calculate the sampling time
  float sample_time = 1000.0/adcConvFreq;
  String s_sample_time(sample_time);

  // Draw the frame
  path = "M0 0 L1000 0 L1000 512 L0 512 Z ";

  // First point
  x = 0;
  y = 512 - (scopeData[0] >> 3);
  path += "M" + String(x) + " " + String(y) + " ";

  // Remaining points
  for (int i = 1; i < LEN_SCOPEDATA; i++) {
    x = i;
    y = 512 - (scopeData[i] >> 3);
    path += "L" + String(x) + " " + String(y) + " ";
  }

  // Create the response
  String response = R"(
    <!DOCTYPE html><html>
      <body>
        <table style="width:800px;"><tr>
          <td style="width:23%;">Sampling frequency</td>
          <td style="width:11%;"><a href="/?freq=20">20 KHz</a></td>
          <td style="width:11%;"><a href="/?freq=50">50 KHz</a></td>
          <td style="width:11%;"><a href="/?freq=100">100 KHz</a></td>
          <td style="width:11%;"><a href="/?freq=200">200 KHz</a></td>
          <td style="width:11%;"><a href="/?freq=500">500 KHz</a></td>
          <td style="width:11%;"><a href="/?freq=1000">1 MHz</a></td>
          <td style="width:11%;"><a href="/?freq=2000">2 MHz</a></td>
        </tr></table>
        <p>Sample time = {0} ms</p>
        <svg height="512" width="1024" xmlns="http://www.w3.org/2000/svg">
          <path style="fill:none;stroke:black;stroke-width:1" d="{1}">
        </svg>
        <p><a href="/framedata" target="_blank">Click here for the raw data</a></p>
      </body>
    </html>
  )";
  response.replace("{0}", s_sample_time);
  response.replace("{1}", path);

  // Send the response
  server.send(200, "text/html", response);
}

// ---------------------------------------------------------------------
// sendFrameData
// -------------
// This sends the raw data from the frame.
// ---------------------------------------------------------------------
void sendFrameData() {
  // Append all the data to a string
  String response = "";
  for (int i = 0; i < LEN_SCOPEDATA; i++) {
    response += String(scopeData[i]) + "\r\n";
  }

  // Send the response
  server.send(200, "text/plain", response);
}

// ---------------------------------------------------------------------
// onRoot
// ------
// The root URL prints a help screen
// ---------------------------------------------------------------------
void onRoot() {
  if (server.hasArg("freq")) {
    int freq = server.arg("freq").toInt();
    if (freq < 20 || freq > 2000) {
      Serial.printf("Requested sampling frequency %d is out of range\n", freq);
    }
    else {
      adcConvFreq = freq;
      initADC(freq);
      Serial.printf("Sampling frequency = %d\n", freq);
    }
  }
  if (getFrame(scopeData, LEN_SCOPEDATA)) {
    sendFrameSVG();
    return;
  }

  // If there was an error getting the frame print an error message
  String response = R"(
    <!DOCTYPE html><html>
      <body>
        <h1>ESPScope</h1>
        <p>Error getting frame</p>
      </body>
    </html>
  )";
  server.send(200, "text/html", response);
}

// ---------------------------------------------------------------------
// onFrameData
// -----------
// Send the raw data
// ---------------------------------------------------------------------
void onFrameData() {
  sendFrameData();
}

// ---------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  // Wait at least a second for the serial interface to connect
  while (!Serial)
    delay(1000);
  delay(1000);
  Serial.println("Starting ESPScope ...");

  // Connect to the wifi
  connectWiFi();
 
  // Set the calibration frequency
  setCalibFreq(calibFreq);

  // Initialise the ADC
  if (!initADC(adcConvFreq)) {
    Serial.println("Error initialising ADC");
    vTaskSuspend(NULL);
  }
  Serial.println("Done initADC");

 // Set the paths we will handle
  server.on("/", onRoot);
  server.on("/framedata", onFrameData);

  // Start the HTTP server
  server.begin();
}

// ---------------------------------------------------------------------
// Main loop
// ---------------------------------------------------------------------
void loop() {
  server.handleClient();
  delay(100);
}