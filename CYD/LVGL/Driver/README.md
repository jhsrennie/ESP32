# Driver

This sketch demonstrates how to use LVGL on the CYD using a hand written display driver. It does the same as the LVGL sketch but without having to include TFT_eSPI to do the rendering.

Why would you want to do this? Well for the CYD you wouldn't. You'd just use TFT_eSPI like everyone else. But if you wanted to adapt this code to a different display controller this code demonstrates the principles involved.

LVGL only requires the user to provide a function to blit a bitmap constructed in memory to the display. The driver code in `esp_lcd_ili9341.c` provides this function `panel_ili9341_draw_bitmap()` as well as some support functions to create the diaplsy object, mirror and rotate the distplay, etc.

All the LVGL code is in `LVGLStuff.cpp`. This is the same as the LVGL code used in the TFT_eSPI example. The code to configure the display is in `Driver.ino`. The only functions you need to look at are:

`InitILI9341()`  
This contains the code to configure the display then create a panel object that is used to transfer data to the display.

`BlitILI9341()`  
This contains the code to transfer bitmap data to the display

In LVGLStuff.cpp the `InitLVGL()` function configures LVGL to use our `BlitILI9341()` function for rendering.