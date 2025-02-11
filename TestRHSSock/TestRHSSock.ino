// ---------------------------------------------------------------------
// TestRHSSock
// -----------
// Demonstrate how to use the CRHSSock class
// ---------------------------------------------------------------------
#include <WiFi.h>
#include <RHSSock.h>

// WiFi credentials
String SSID = "MySSID";
String PWD  = "MyPassword";

// ---------------------------------------------------------------------
// ConnectWiFi
// ---------------------------------------------------------------------
void ConnectWiFi() {
  WiFi.begin(SSID, PWD);
  int loopcnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf("Connecting: time %d, WiFi status = %d, signal = %d\n", loopcnt++, WiFi.status(), WiFi.RSSI());
    delay(1000);
  }
  Serial.printf("Connected: %s\n", WiFi.localIP().toString().c_str());
}

// ---------------------------------------------------------------------
// ConnectToServer
// ---------------
// Test connecting to a server. This connects, sends a string and
// disconnects again
// ---------------------------------------------------------------------
void ConnectToServer(const char* Peer, int Port) {
  CRHSSocket sock;

  Serial.printf("Connecting to %s on port %d ... ", Peer, Port);
  if (!sock.Connect(Peer, Port)) {
    Serial.printf("failed: %s\n", sock.GetLastErrorMsg());
    return;
  }
  Serial.printf("connected!\n");

  char* hello = "Hello from RHSSock\r\n";
  for (int i = 0; i < 10; i++) {
    sock.puts(hello);
  }

  sock.CloseSocket();
}

// ---------------------------------------------------------------------
// TelnetServer
// ------------
// This implements a simple telnet server
// ---------------------------------------------------------------------
void TelnetServer() {
  CRHSSocket sock;

  // Listen for a connection on port 23
  if (!sock.Listen("0.0.0.0", 23)) {
    Serial.printf("Listen failed: %s\n", sock.GetLastErrorMsg());
    return;
  }

  // Wait for a connection
  char peername[256];
  CRHSSocket* peer = sock.Accept(peername, 255);
  if (!peer) {
    Serial.printf("Accept failed: %s\n", sock.GetLastErrorMsg());
    return;
  }
  Serial.printf("Accepted connection from %s\n", peername);

  // Read commands from the client
  char cmdbuf[256];
  while (true) {
    char* cmd = peer->gets(cmdbuf, 255);
    if (!cmd) {
      Serial.printf("gets failed: %s\n", sock.GetLastErrorMsg());
      return;
    }

    // Exit
    if (strcmp(cmd, "exit") == 0) {
      peer->puts("Bye!\r\n");
      break;
    }

      // Unrecognised command
    else {
      peer->printf("Unrecognised command: %s\r\n", cmd);
    }
  }

  peer->CloseSocket();
  sock.CloseSocket();
}

// ---------------------------------------------------------------------
// setup
// ---------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(1000);

  // Connect the WiFi
  Serial.println("Connecting wifi ...");
  ConnectWiFi();
}

// ---------------------------------------------------------------------
// loop
// ---------------------------------------------------------------------
void loop() {
  // Listen for telnet connections
  TelnetServer();
  delay(1000);
}
