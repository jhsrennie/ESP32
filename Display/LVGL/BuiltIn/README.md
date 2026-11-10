# BuiltIn

This demonstrates how to use the built in ILI9341 driver provided by LVGL on an ESP32.

Note that I had to use:
```
#define LV_COLOR_FORMAT_DEFAULT LV_COLOR_FORMAT_RGB565_SWAPPED
```
in `lv_conf.h` so set the pixel data to bigendian. This is not normally necessary if you are using a library like TFT_eSPI as the library does it for you.

[The Espressif documentation for the ILI9341 built in driver is here](https://lvgl.io/docs/open/integration/external_display_controllers/ili9341).