# ServeJPG

This is a simple example of how to use my esp_new_jpeg Arduino library.

In my sketch the `generateFrame()` function creates a greyscale bitmap as an array, then calls the library functions to encode it as a bitmap. My code creates a bitmap consisting of vertical stripes because it was easy. You could modify the code to create more interesting bitmaps as you wish.

The bitmap I use is greyscale with one byte per pixel, so I specify the format for the encode in the `setup()` function as:

```
  jpeg_enc_cfg.src_type = JPEG_PIXEL_FORMAT_GRAY;
```

If you look at the header file `esp_jpeg_common.h` in the library you can see the other supported formats e.g. `JPEG_PIXEL_FORMAT_RGB888` for an RGB bitmap with three bytes per pixel.

I had to think of something to do with the jpeg once I had encoded it, so I decided to use a web server so the user can download it. The sketch connects to Wi-Fi then starts a web server. When you open the web server in your preferred browser it will send the jpeg so you can view and/or save it. You will need to replace the Wi-Fi credentials at line 10 with your own SSID and password.

I used an S3 N16R8 with 8MB of PSRAM to test this. It will compile for ESP32s with no PSRAM but you'll need to reduce the size of the bitmap or there won't be enough memory for the buffers required. Assuming you have a board with PSRAM remember to enable it from the Arduino IDE _Tools_ menu.