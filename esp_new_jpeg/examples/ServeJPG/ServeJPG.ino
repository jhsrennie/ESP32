//----------------------------------------------------------------------
// ServeJPG
// ========
// Test of the ESP_NEW_JPEG encoder
// This creates a web server that sends a JPG to the client
//----------------------------------------------------------------------
#include <WiFi.h>
#include <esp_jpeg_enc.h>
#include <esp_http_server.h>

// Replace with your network credentials
const char* ssid = "SSID";
const char* password = "PASS";

// Global handle to the HTTP server
httpd_handle_t h_httpd = NULL;

// Image size
#define JPG_WIDTH 320
#define JPG_DEPTH 240

// Buffer used to hold input data
// This is a greyscale image so it has 1 byte per pixel
#define INBUF_SIZE (JPG_WIDTH*JPG_DEPTH)
uint8_t* in_buf = NULL;

// Buffer to hold the jpeg
#define JPGBUF_SIZE (JPG_WIDTH*JPG_DEPTH)
uint8_t* jpg_buf = NULL;

// Encoder handle
jpeg_enc_handle_t jpeg_enc = NULL;

//----------------------------------------------------------------------
// generateFrame
// -------------
// This creates a bitmap in the input buffer then encodes it into the
// jpg buffer.
//----------------------------------------------------------------------
void generateFrame(uint8_t *out_buf, int outbuf_size, int *out_size) {
  // Set the input to vertical bars 16 pixels wide
  for (int y = 0; y < JPG_DEPTH; y++) {
    for (int x = 0; x < JPG_WIDTH; x++) {
      int offset = y*JPG_WIDTH + x;
      in_buf[offset] = (x >> 4) % 2 == 0 ? 0 : 0xFF;
    }
  }

  // Encode the image
  jpeg_error_t ret = jpeg_enc_process(jpeg_enc, in_buf, INBUF_SIZE, out_buf, outbuf_size, out_size);
  if (ret != JPEG_ERR_OK)
    Serial.printf("Encoding error: %d\n", ret);
}

//----------------------------------------------------------------------
// jpg_handler
// -----------
// Function that sends the jpg
//----------------------------------------------------------------------
esp_err_t jpg_handler(httpd_req_t *req) {
  esp_err_t res = ESP_OK;
  int jpg_buf_len = 0;

  Serial.println("Received request for /");

  // Generate the next frame
  generateFrame(jpg_buf, INBUF_SIZE, &jpg_buf_len);

  // Set the response type to jpeg
  httpd_resp_set_type(req, "image/jpeg");

  // Send the image
  res = httpd_resp_send(req, (const char*) jpg_buf, jpg_buf_len);
  Serial.printf("Finished request: %d\n", res);

  return res;
}

//----------------------------------------------------------------------
// startWebServer
// --------------
// Configure and start the web server
//----------------------------------------------------------------------
void startWebServer(){
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;

  // Start te web server
  Serial.printf("Starting web server on port: '%d'\n", config.server_port);
  if (httpd_start(&h_httpd, &config) != ESP_OK) {
    Serial.println("Failed to start web server");
    vTaskSuspend(NULL);
  }

  // Configure the handler for the jpg
  httpd_uri_t uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = jpg_handler,
    .user_ctx  = NULL
  };
  httpd_register_uri_handler(h_httpd, &uri);
}

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  // Wi-Fi connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Allocate the input buffer. This has to be 16 byte aligned
  in_buf = (uint8_t*) jpeg_calloc_align(INBUF_SIZE, 16);
  if (!in_buf) {
    Serial.println("Failed to allocate input buffer");
    vTaskSuspend(NULL);
  }

  // Allocate the output buffer
  jpg_buf = (uint8_t*) jpeg_calloc_align(JPGBUF_SIZE, 16);
  if (!jpg_buf) {
    Serial.println("Failed to allocate output buffer");
    vTaskSuspend(NULL);
  }

  // Initialise the encoder
  jpeg_enc_config_t jpeg_enc_cfg = DEFAULT_JPEG_ENC_CONFIG();
  jpeg_enc_cfg.width = JPG_WIDTH;
  jpeg_enc_cfg.height = JPG_DEPTH;
  jpeg_enc_cfg.src_type = JPEG_PIXEL_FORMAT_GRAY;

  jpeg_error_t ret = jpeg_enc_open(&jpeg_enc_cfg, &jpeg_enc);
  if (ret != JPEG_ERR_OK) {
    Serial.printf("jpeg_enc_open failed: %d\n", ret);
    vTaskSuspend(NULL);
  }

  // Start the web server
  startWebServer();
  Serial.print("Server ready! Go to: http://");
  Serial.println(WiFi.localIP());
}

//----------------------------------------------------------------------
// loop
// ----
// The loop isn't used for anything as the HTTP server does all the work
//----------------------------------------------------------------------
void loop() {
  delay(100);
}