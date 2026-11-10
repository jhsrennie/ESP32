# LovyanGFX

This demonstrates how to use LovyanGFX as the display driver for LVGL.

This example uses a cheap yellow display and a suitable `lv_lgfx_user.hpp` file is included to configure Lovyan to use the CYD.

The sketch is as simple as possible to keep it easy to see how it works. It just creates an edit control filling the screen. Any text the user types into the serial monitor is added to the edit control.

To make LVGL use LovyanGFX as the display driver you call `lv_init()` as usual then call:
```
  disp = lv_lovyan_gfx_create(SCREEN_WIDTH, SCREEN_HEIGHT, drawBuf, sizeof(drawBuf), true);
```
to create the display object, where `drawBuf` is a `uint8_t` array that you have allocated.

You also need to edit `lv_conf.h`. Find the line:
```
#define LV_USE_LOVYAN_GFX 1
```
and change the `0` to `1`.