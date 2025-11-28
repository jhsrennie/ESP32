// ---------------------------------------------------------------------
// ESPWaveGen
// ==========
// A wave generator for the ESP32
// ---------------------------------------------------------------------

#include <WebServer.h>

// Prototypes
void connectWiFi();
bool InitWaveGen(int Type, int Frequency);

// Use a global variable for the server since there is only one server
WebServer server(80);

// Global variables for the wave type and frequency
int waveType, waveFreq;

// ---------------------------------------------------------------------
// onRoot
// ------
// This shows the control page
// ---------------------------------------------------------------------
void onRoot() {
  // Check for form arguments
  int type = 0; // Default type is sine
  if (server.hasArg("type")) {
    type = server.arg("type").toInt();
    if (type < 1 || type > 3)
      type = 0;
  }

  int freq = 1000; // Default frequency is 1 KHz
  if (server.hasArg("freq")) {
    freq = server.arg("freq").toInt();
    // Allowed frequency range is 40Hz to 200KHz
    if (freq < 40 || freq > 200000)
      freq = 1000;
  }

  Serial.printf("Type = %d, frequency = %d\n", type, freq);

  // If the change succeeds save the type and frequency
  if (InitWaveGen(type, freq)) {
    waveType = type;
    waveFreq = freq;
  }

  // Send the response
  String response = R"(
    <!DOCTYPE html><html>
      <body>
        <h1>ESPWaveGen</h1>
        <p>Frequency range is 40Hz to 200KHz</p>
        <form action="" method="get">
        <table style="width:400px;">
          <tr>
            <td style="width:50%;">Waveform type</td>
            <td style="width:50%;">
              <select name="type" style="width:100%;">
                <option value="0" {0}>Sine</option>
                <option value="1" {1}>Triangle</option>
                <option value="2" {2}>Sawtooth</option>
                <option value="3" {3}>Square</option>
              </select>
            </td>
          </tr>
          <tr>
            <td>Frequency (/Hz)</td>
            <td><input type="text" name="freq" value={4} style="width:100%;"></td>
          </tr>
          <tr>
            <td colspan="2"><input type="submit" value="Go"></td>
          </tr>
        </table>
        </form>
      </body>
    </html>
  )";
  // Set the form wave type
  switch (type) {
    case 1:
      response.replace("{1}", "selected");
      break;
    case 2:
      response.replace("{2}", "selected");
      break;
    case 3:
      response.replace("{3}", "selected");
      break;
    default:
      response.replace("{0}", "selected");
      break;
  }
  // Set the form frequency
  response.replace("{4}", String(freq));
  // Send the page
  server.send(200, "text/html", response);
}

// ---------------------------------------------------------------------
// CheckSerialInput
// ----------------
// Check for commands typed at the serial monitor
// ---------------------------------------------------------------------
void CheckSerialInput() {
  // Static buffer for getting commands
  String buf = "";

  // Check for input
  while (Serial.available()) {
    char c = Serial.read();
    // Line feed terminates a command
    if (c != 10) {
      buf += c;
    }
    else {
      // If we have a number set the frequency
      int freq = buf.toInt();
      if (freq != 0) {
        if (freq < 40 || freq > 200000) {
          Serial.printf("Frequency out of range: %d\n", freq);
        }
        else {
          if (InitWaveGen(waveType, freq))
            waveFreq = freq;
        }
        return;
      }
      // Else we have a command
      else {
        int type;
        if (buf == "sine") {
          type = 0;
        }
        else if (buf == "triangle") {
          type = 1;
        }
        else if (buf == "sawtooth") {
          type = 2;
        }
        else if (buf == "square") {
          type = 3;
        }
        else {
          Serial.printf("Unknown command: %s\n", buf.c_str());
          return;
        }
        if (InitWaveGen(type, waveFreq))
          waveType = type;
      }
      // Clear the buffer
      buf = "";
    }
  }
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
  Serial.println("Starting ESPWaveGen ...");

  // Connect to the wifi
  connectWiFi();
 
 // Set the paths we will handle
  server.on("/", onRoot);

  // Start the HTTP server
  server.begin();

  // Start the wave generator with a 1KHz sine wave
  InitWaveGen(0, 1000);
}

// ---------------------------------------------------------------------
// Main loop
// ---------------------------------------------------------------------
void loop() {
  // Process web server messages
  server.handleClient();
  delay(100);

  // Check for command on the serial port
  if (Serial.available())
    CheckSerialInput();
}