//----------------------------------------------------------------------
// websock.cpp
// -----------
// This contains all the WebSockets code
//----------------------------------------------------------------------
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "webvars.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// Data to send
// This is a number that increments every time a client requests it
// We send it as JSON e.g. "{"foo": 123}"
int foo = 0;
String fooJSON = R"({"foo": <foo>})";

// Send the data to all connected clients
void notifyClients() {
  String s = fooJSON;
  s.replace("<foo>", String(foo));
  ws.textAll(s);
}

// Handle messages from clients
// In this simple example we ignore the message details and always
// respond by sending the current data.
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo* info = (AwsFrameInfo*) arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    foo++;
    notifyClients();
  }
}

// Handle WebSocket events
// In this simple example we only need to respond to the WS_EVT_DATA
// event that indicates a message from a client has been received.
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

// Initialise the WebSocket
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  // Web server URLs
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", homepage);
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/javascript", script);
  });

  // Start server
  server.begin();
}