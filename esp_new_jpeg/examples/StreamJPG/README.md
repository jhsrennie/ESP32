# StreamJPG

This is the reason I created the esp_new_jpeg Arduino library. I had an MLX90640 infra-red camera and I wanted to use it like a webcam. The MLX90640 records images as greyscale bitmaps, so to make it work as a webcam I had to grab the bitmaps, encode them into JPEG format then stream them out using a web server. This is what this sketch does, though since we don't all have cameras it makes up an image of animated vertical stripes.

The `generateFrame()` function creates a bitmap with vertical stripes then encodes it using the esp_new_jpec library. Every time this function is called it creates the next frame. To view the stream the sketch creates a web server, and you just connect to this web server to view the stream. Just like a webcam!

This code is very similar to the ServerJPG example. The only major difference is that the web server serves a stream instead of a single image. You will need to replace the Wi-Fi credentials at line 11 with your own SSID and password.

I used an S3 N16R8 with 8MB of PSRAM to test this. It will compile for ESP32s with no PSRAM but you'll need to reduce the size of the bitmap or there won't be enough memory for the buffers required. Assuming you have a board with PSRAM remember to enable it from the Arduino IDE _Tools_ menu.