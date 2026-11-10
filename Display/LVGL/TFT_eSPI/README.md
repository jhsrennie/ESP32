# TFT_eSPI

This demonstrates how to use TFT_eSPI as the display driver for LVGL.

This example uses a cheap yellow display and a suitable TFT_eSPI `User_Setup.h` file is included. Copy the `User_Setup.h` file into your `libraries/TFT_eSPI` directory.

The sketch is as simple as possible to keep it easy to see how it works. It just creates an edit control filling the screen. Any text the user types into the serial monitor is added to the edit control.

To make LVGL use TFT_eSPI as the display driver you call `lv_init()` as usual then call:
```
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, drawBuf, sizeof(drawBuf));
```
to create the display object, where `drawBuf` is a `uint8_t` array that you have allocated.

You also need to edit `lv_conf.h`. Find the line:
```
#define LV_USE_TFT_ESPI 0
```
and change the `0` to `1`.