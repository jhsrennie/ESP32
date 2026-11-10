# LVGL examples

This directory contains examples of using the LVGL graphics library.

LVGL does not include a display driver so your code needs to implement a display driver. The easiest way to do this is to use the TFT_eSPI or LovyanGFX libraries as the display driver since those libraries can do all the hard work for you. However recent versions of LVGL added support for using the display directly. Alternatively you can use a user written `flush_cb()` function. This directory contains examples showing how to do all of these.

### BuiltIn
Use the LVGL built in ILI9341 display driver

### FlushCB
Use a `flush_cb()` callback function and a user written display driver

### LovyanGFX  
Use the LovyanGFX library as the display driver

### TFT_eSPI  
Use the tFT_eSPI library as the display driver