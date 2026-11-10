# FlushCB

LVGL can be configured to use a user supplied `flush_cb()` function to render its output to the display. You would typically do this if you are writing your own display driver to write pixels to the display. This sketch shows how to use a `flush_cb()` function.

The display driver code is in `ILI9341.cpp`. It is written for an ILI9341 display and uses the low level SPI commands to communicate with the display. It implements two functions for the sketch to use:

LCDInit()  
Called once to initialise the driver

LCDFlush  
The `flush_cb()` function for LVGL to use

To use a different display you need to replace `ILI9341.cpp` and `.h` with the corresponding code to drive your display.

Note that I had to use:
```
#define LV_COLOR_FORMAT_DEFAULT LV_COLOR_FORMAT_RGB565_SWAPPED
```
in `lv_conf.h` so set the pixel data to bigendian. This is not normally necessary if you are using a library like TFT_eSPI as the library does it for you.

[The Espressif documentation for using a `flush_cb()` function is here](https://lvgl.io/docs/open/main-modules/display/setup).