//----------------------------------------------------------------------
// StreamJPG
// =========
// This streams an animation by creating a bitmap and encoding it as
// JPEG then streaming it to an HTTP client.
//----------------------------------------------------------------------
#include <WiFi.h>
#include <esp_jpeg_enc.h>
#include <esp_http_server.h>

// Replace with your network credentials
const char* ssid = "SSID";
const char* password = "PASS";

// Various MIME encoding constants
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// Global handle to the HTTP server
httpd_handle_t stream_httpd = NULL;

// Buffer used to hold input data
#define JPG_WIDTH 320
#define JPG_DEPTH 240
#define INBUF_SIZE (JPG_WIDTH*JPG_DEPTH)
uint8_t* in_buf = NULL;

// Encoder handle
jpeg_enc_handle_t jpeg_enc = NULL;

//----------------------------------------------------------------------
// generateFrame
// -------------
//----------------------------------------------------------------------
void generateFrame(uint8_t *out_buf, int outbuf_size, int *out_size) {
  static int frame = 0;

  // Set the input to vertical bars 16 pixels wide
  for (int y = 0; y < JPG_DEPTH; y++) {
    for (int x = 0; x < JPG_WIDTH; x++) {
      int offset = y*JPG_WIDTH + x;
      in_buf[offset] = ((x + frame) >> 4) % 2 == 0 ? 0 : 0xFF;
    }
  }
  frame++;

  // Encode the image
  jpeg_error_t ret = jpeg_enc_process(jpeg_enc, in_buf, INBUF_SIZE, out_buf, outbuf_size, out_size);
  if (ret != JPEG_ERR_OK)
    Serial.printf("Encoding error: %d\n", ret);
}

//----------------------------------------------------------------------
// stream_handler
// --------------
// Function that streams the video
//----------------------------------------------------------------------
#define JPGBUF_SIZE 0x8000 // (JPG_WIDTH*JPG_DEPTH)

esp_err_t stream_handler(httpd_req_t *req){
  esp_err_t res = ESP_OK;
  int jpg_buf_len = 0;
  uint8_t* jpg_buf = NULL;
  char part_buf[64];

  // Allocate the output buffer
  jpg_buf = (uint8_t*) jpeg_calloc_align(JPGBUF_SIZE, 16);
  if (!jpg_buf) {
    Serial.println("Failed to allocate output buffer");
    vTaskSuspend(NULL);
  }

  // Set the response type to MIME
  httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);

  // The function loops continuously sending frames
  int num_frames = 0;
  int last_sec = millis();

  while(true) {
    // Generate the next frame
    generateFrame(jpg_buf, JPGBUF_SIZE, &jpg_buf_len);

    // Just for interest we count the frames per second
    num_frames++;
    if (millis() - last_sec >= 1000) {
      Serial.printf("fps = %d\n", num_frames);
      num_frames = 0;
      last_sec = millis();
    }

    // Send the content length
    size_t hlen = snprintf((char*)part_buf, 64, _STREAM_PART, jpg_buf_len);
    res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    if (res != ESP_OK) {
      Serial.printf("Error sending content length: %d\n", res);
      break;
    }

    // Send the image
    res = httpd_resp_send_chunk(req, (const char *) jpg_buf, jpg_buf_len);
    if (res != ESP_OK) {
      Serial.printf("Error sending image: %d\n", res);
      break;
    }

    // Send the MIME boundary
    res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    if (res != ESP_OK) {
      Serial.printf("Error sending boundary: %d\n", res);
      break;
    }
  }

  // We only reach here if there is an error
  jpeg_free_align(jpg_buf);
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

  Serial.printf("Starting web server on port: '%d'\n", config.server_port);
  esp_err_t res = httpd_start(&stream_httpd, &config);
  if (res != ESP_OK) {
    Serial.printf("Error starting web server: %d\n", res);
    vTaskSuspend(NULL);
  }

  httpd_uri_t uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = stream_handler,
    .user_ctx  = NULL
  };
  httpd_register_uri_handler(stream_httpd, &uri);
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

  // Initialise the encoder
  jpeg_enc_config_t jpeg_enc_cfg = DEFAULT_JPEG_ENC_CONFIG();
  jpeg_enc_cfg.width = JPG_WIDTH;
  jpeg_enc_cfg.height = JPG_DEPTH;
  jpeg_enc_cfg.src_type = JPEG_PIXEL_FORMAT_GRAY;

  jpeg_enc = NULL;
  jpeg_error_t ret = jpeg_enc_open(&jpeg_enc_cfg, &jpeg_enc);
  if (ret != JPEG_ERR_OK) {
    Serial.printf("jpeg_enc_open failed: %d\n", ret);
    vTaskSuspend(NULL);
  }
  Serial.println("Encoder initialised");

  // Start streaming web server
  startWebServer();

  // Setup done
  Serial.print("Camera Stream Ready! Go to: http://");
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