# LVGL-IDF

This sketch demonstrates how to use LVGL on the CYD using the LCD functions built into the ESP-IDF. It does the same as the LVGL sketch but without having to include TFT_eSPI to do the rendering.

Why would you want to do this? Because you can :-)

LVGL only requires the user to provide a function to blit a bitmap constructed in memory to the display. ESP-IDF provides a function [esp_lcd_panel_draw_bitmap()](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/lcd/index.html) to do this along with a few support functions to initialise the LCD, rotate and mirror the display etc. It has built in support for a few display controllers including the InitST7789/ILI9341 used in the CYD.

All the LVGL code is in `LVGLStuff.cpp`. This is the same as the LVGL code used in the TFT_eSPI example. The code to configure the display is in `ESP-IDF.ino`. The only functions you need to look at are:

`InitST7789()`  
This contains the ESP-IDF code to configure the display then create a panel object that is used to transfer data to the display.

`BlitST7789()`  
This contains the code to transfer bitmap data to the display

In LVGLStuff.cpp the `InitLVGL()` function configures LVGL to use our `BlitST7789()` function for rendering.