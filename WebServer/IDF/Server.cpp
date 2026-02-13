//----------------------------------------------------------------------
// Server.cpp
// ==========
// Web server code
//----------------------------------------------------------------------
#include <esp_http_server.h>
#include <stdlib.h>
// We include the Arduino header for the String type because the
// String.replace() is more convenient than string.replace(). You
// probably wouldn't do this in a real live app.
#include <Arduino.h>

// Global handle to the HTTP server
static httpd_handle_t stream_httpd = NULL;

// Size of the graph
#define GRAPH_WIDTH 800
#define GRAPH_DEPTH 500

// Number of data points to plot
#define NUM_DATA    800

// ---------------------------------------------------------------------
// Parts of the web page
// ---------------------------------------------------------------------
static const String htmlFrame = R"(
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
static esp_err_t onRoot(httpd_req_t *req) {
  String response = htmlFrame;

  // See if form data was supplied
  // This returns the whole form data string undecoded. ESP-DF doesn't
  // have  built in way to get a single form variable.
  char* query_str = NULL;
  int query_len = httpd_req_get_url_query_len(req);
  if (query_len > 0) {
    query_str = new char[query_len+1];
    httpd_req_get_url_query_str(req, query_str, query_len+1);
  }

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
  if (query_str) {
    body += "<p>You entered \"";
    body += query_str;
    body += "\" into the form</p>";
    delete query_str;
  }
  response.replace("{body}", body);

  // Send the response
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, response.c_str(), response.length());

  // Return indicating success
  return ESP_OK;
}

// ---------------------------------------------------------------------
// onSVG
// -----
// Display a graph using SVG
// ---------------------------------------------------------------------
static esp_err_t onSVG(httpd_req_t *req) {
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
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, response.c_str(), response.length());

  // Return indicating success
  return ESP_OK;
}

// ---------------------------------------------------------------------
// onTextFile
// ----------
// Send a text file
// ---------------------------------------------------------------------
static esp_err_t onTextFile(httpd_req_t *req) {
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
  httpd_resp_set_type(req, "text/plain");
  httpd_resp_send(req, response.c_str(), response.length());

  // Return indicating success
  return ESP_OK;
}

//----------------------------------------------------------------------
// ServerSetup
// -----------
// Configure the web server
//----------------------------------------------------------------------
bool ServerSetup() {
  // Set up the paths we accept
  httpd_uri_t uri_root = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = onRoot,
    .user_ctx  = NULL
  };

  httpd_uri_t uri_svg = {
    .uri       = "/svg",
    .method    = HTTP_GET,
    .handler   = onSVG,
    .user_ctx  = NULL
  };

  httpd_uri_t uri_textfile = {
    .uri       = "/textfile",
    .method    = HTTP_GET,
    .handler   = onTextFile,
    .user_ctx  = NULL
  };

  // Start the server
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  if (httpd_start(&stream_httpd, &config) != ESP_OK) {
    return false;
  }

  // Register the paths
  httpd_register_uri_handler(stream_httpd, &uri_root);
  httpd_register_uri_handler(stream_httpd, &uri_svg);
  httpd_register_uri_handler(stream_httpd, &uri_textfile);

  // Return indicating success
  return true;
}