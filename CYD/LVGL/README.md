# LVGL

These apps are experiments in using the LVGL library in different ways.

LVGL is a high level library and requires the user to provide a function to blit bitmaps to the screen. Tpically you need to call the LVGL function:

```
lv_display_set_flush_cb(display, flush_cb);
```

to set the blit function, where `flush_cb` is a function:

```
void flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map);
```

Often the TFT_eSPI library is used to provide the `flush_cb()` function, but you can use the LCD functions built into the ESP-IDF or you can write your own driver to control the display directly. The subdirectories provide examples of these three approaches. In fact it turns out that using TFT_eSPI is only a small overhead and it's certainly the simplest option.

TFT_eSPI  
Shows how to use LVGL with the TFT_eSPI library

ESP-IDF  
Shows how to use the ESP-IDF LCD functions

Driver  
Shows how to use your own driver