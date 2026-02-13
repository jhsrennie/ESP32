//----------------------------------------------------------------------
// Server.cpp
// ==========
// Web server code
//----------------------------------------------------------------------
#include <WebServer.h>

// Use a global variable for the server since there is only one server
WebServer server(80);

// Size of the graph
#define GRAPH_WIDTH 800
#define GRAPH_DEPTH 500

// Number of data points to plot
#define NUM_DATA    800

// ---------------------------------------------------------------------
// Parts of the web page
// ---------------------------------------------------------------------
const String htmlFrame = R"(
<!DOCTYPE html>
<html lang="en">
  <head>
    <title>ESP32 Web Test</title>
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
        <td class="header">ESP32 Web Test</td>
      </tr>
      <tr><td class="blank"></td></tr>
      <tr>
        <td>
        {body}
        </td>
      </tr>
      <tr><td class="blank"></td></tr>
      <tr>
        <td class="subheader">Copyright A. N. Other 2026</td>
      </tr>
    </table>
  </body>
</html>
  )";

// ---------------------------------------------------------------------
// onRoot
// ------
// ---------------------------------------------------------------------
void onRoot() {
  String response = htmlFrame;

  // See if form data was supplied
  String formtext = "";
  if (server.hasArg("formtext"))
    formtext = server.arg("formtext");

  // Build the response
  String body = R"(
<p>Click <a href="/svg" target="_blank">here</a> to view a graph</p>
<p>Click <a href="/textfile" target="_blank">here</a> to download a text file</p>
<h3>Example form</h3>
<form action="/" method="GET">
  <p>Type some text:</p>
  <input type="text" name="formtext"><br/>
  <input type="submit" value="Click Me">
</form>
  )";
  if (formtext != "") {
    body += "<p>You entered \"" + formtext + "\" into the form</p>";
  }
  response.replace("{body}", body);

  // Send the response
  server.send(200, "text/html", response);
}

// ---------------------------------------------------------------------
// onSVG
// -----
// Display a graph using SVG
// ---------------------------------------------------------------------
static void onSVG() {
  // Make up some data
  uint16_t data[NUM_DATA];
  for (int i = 0; i < NUM_DATA; i++)
    data[i] = (i % 100) + 10;

  // Y scale is 0 to 120
  int y_min = 0;
  int y_max = 120;

  // Convert a y value to position in the graph
  #define YPOS(y) (GRAPH_DEPTH - (GRAPH_DEPTH*(y - y_min))/y_max)

  // Draw the frame
  String frame = "M0 0 ";
  frame += "L" + String(GRAPH_WIDTH) +" 0 ";
  frame += "L" + String(GRAPH_WIDTH) + " " + String(GRAPH_DEPTH) + " ";
  frame += "L0 " + String(GRAPH_DEPTH) + " Z ";

  // First point
  int x = 0;
  int y = YPOS(data[0]);
  String line = "M" + String(x) + " " + String(y) + " ";

  // Remaining points
  for (int i = 1; i < NUM_DATA; i++) {
    x = (GRAPH_WIDTH*i)/NUM_DATA;
    y = YPOS(data[i]);
    line += "L" + String(x) + " " + String(y) + " ";
  }

  // Build the response
  String svg = R"(
<svg width="800" height="500" xmlns="http://www.w3.org/2000/svg">
  <path style="fill:none;stroke:black;stroke-width:1" d="{1}"/>
  <path style="fill:none;stroke:red;stroke-width:1" d="{2}"/>
</svg>
  )";
  svg.replace("{1}", frame);
  svg.replace("{2}", line);
  
  String response = htmlFrame;
  response.replace("{body}", svg);

  // Send the response
  server.send(200, "text/html", response);
}

// ---------------------------------------------------------------------
// onTextFile
// ----------
// Send a text file
// ---------------------------------------------------------------------
void onTextFile() {
  // Make up some data
  uint16_t data[NUM_DATA];
  for (int i = 0; i < NUM_DATA; i++)
    data[i] = (i % 100) + 10;

  // Append all the data to a string
  String response = "Time (/ms)\tData\r\n";
  for (int i = 0; i < NUM_DATA; i++) {
    response += String(i) + "\t" + String(data[i]) + "\r\n";
  }

  // Send the response
  server.send(200, "text/plain", response);
}

//----------------------------------------------------------------------
// ServerSetup
// -----------
// Configure the web server
//----------------------------------------------------------------------
bool ServerSetup() {
 // Set the paths we will handle
  server.on("/", onRoot);
  server.on("/svg", onSVG);
  server.on("/textfile", onTextFile);

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