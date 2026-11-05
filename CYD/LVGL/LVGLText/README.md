# LVGLText

This demonstrates how to use TFT_eSPI ad LovyanGFX as the display drivers for LVGL.

The sketch is as simple as possible to keep it easy to see how it works. It just creates an edit control filling the screen.

To select the library edit the code and uncomment one of the options:
```
// #define USE_TFT_ESPI 1
// #define USE_LOVYANGFX 1
```

You will need to edit `lv_conf.h` and edit one of the lines:
```
#define LV_USE_LOVYAN_GFX 0
#define LV_USE_TFT_ESPI 0
```
and change the `0` to `1`. This tells LVGL that you want to include the code for that driver.