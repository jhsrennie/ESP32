//----------------------------------------------------------------------
// Server.cpp
// ==========
// Web server code
//----------------------------------------------------------------------
#include <WebServer.h>
#include "Data.h"

#define GRAPH_WIDTH 1000
#define GRAPH_DEPTH  600

// Use a global variable for the server since there is only one server
WebServer server(80);

// ---------------------------------------------------------------------
// sendSVG
// -------
// ---------------------------------------------------------------------
void sendSVG() {
  uint16_t data[MAX_ADCDATA];
  int numdata = DataRead(data);
  if (numdata == 0) {
    String response = "No data available";
    server.send(200, "text/plain", response);
    return;
  }

  // String response = "Not implemented yet";
  // server.send(200, "text/plain", response);
  // Draw the frame
  String frame = "M0 0 ";
  frame += "L" + String(GRAPH_WIDTH) +" 0 ";
  frame += "L" + String(GRAPH_WIDTH) + " " + String(GRAPH_DEPTH) + " ";
  frame += "L0 " + String(GRAPH_DEPTH) + " Z ";

  // First point
  int x = 0;
  int y = GRAPH_DEPTH - (data[0]*GRAPH_DEPTH)/4096;
  String path = "M" + String(x) + " " + String(y) + " ";

  // Remaining points
  for (int i = 1; i < MAX_ADCDATA; i++) {
    x = (GRAPH_WIDTH*i)/MAX_ADCDATA;
    y = GRAPH_DEPTH - data[i]*GRAPH_DEPTH/4096;
    path += "L" + String(x) + " " + String(y) + " ";
  }

  String response = R"(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta http-equiv="refresh" content="4">
    <style>
      table {
        border-spacing: 0;
        font-family: Arial, Helvetica, sans-serif;
      }
      td {
        text-align:center;
      }
      td.header {
        height: 3em;
        font-size: 2em;
        font-weight: bold;
        color: white;
        background-color: rgb(49, 62, 78);
      }
      td.subheader {
        height: 3em;
        color: white;
        background-color: rgb(142, 142, 0);
      }
      td.blank {
        height: 1em;
        background-color: white;
      }
    </style>
  </head>
  <body>
    <table style="width:1040px">
      <tr>
        <td class="header">AD8232 Heart Monitor</td>
      </tr>
      <tr><td class="blank"></td></tr>
      <tr>
        <td>
        <svg width="1000" height="600" xmlns="http://www.w3.org/2000/svg">
          <path style="fill:none;stroke:black;stroke-width:1" d="{1}"/>
          <path style="fill:none;stroke:red;stroke-width:1" d="{2}"/>
        </svg>
        </td>
      </tr>
      <tr><td class="blank"></td></tr>
      <tr>
        <td class="subheader"><a href="/data" target="_blank">Click here for the raw data</a></td>
      </tr>
    </table>
  </body>
</html>
  )";

  response.replace("{1}", frame);
  response.replace("{2}", path);

  // Send the response
  server.send(200, "text/html", response);
}

// ---------------------------------------------------------------------
// sendData
// --------
// This sends the raw data from the frame.
// ---------------------------------------------------------------------
void sendData() {
  uint16_t data[MAX_ADCDATA];
  int numdata = DataRead(data);
  if (numdata == 0) {
    String response = "No data available";
    server.send(200, "text/plain", response);
    return;
  }

  // Append all the data to a string
  String response = "Time (/ms)\tData\r\n";
  for (int i = 0; i < numdata; i++) {
    response += String(i*ADC_INTERVAL) + "\t" + String(data[i]) + "\r\n";
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
  // And send the data to the client
   sendSVG();
}

// ---------------------------------------------------------------------
// onData
// ------
// Send the raw data
// ---------------------------------------------------------------------
void onData() {
  sendData();
}

//----------------------------------------------------------------------
// ServerSetup
// -----------
// Configure the web server
//----------------------------------------------------------------------
bool ServerSetup() {
 // Set the paths we will handle
  server.on("/", onRoot);
  server.on("/data", onData);

  // Start the HTTP server
  server.begin();

  // Return indicating success
  return true;
}

//----------------------------------------------------------------------
// ServerLoop
// ----------
// Call this from the loop() function to respond to client requests
//----------------------------------------------------------------------
void ServerLoop() {
  server.handleClient();
}