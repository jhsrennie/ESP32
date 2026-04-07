# LCD

Connecting displays to an ESP32 can be a messy business as there are many different types of display and they all work in different ways. In an operating system like Windows the OS abstracts away all the messy details so for example you can use the Win32 function `LineTo()` to draw a line without worrying what graphics card the PC has.

However the OS on the ESP32 is so cut down that it does not have the graphics support that heavyweight OSs provide. Instead your choices are to either use a graphics library like TFT_eSPI and LVGL, or to drive the display directly. The `GraphicsLibTest` sketch demonstrates the use of graphics libraries while the sketches in this directory explore driving the display directly.

Note that typically the only functions the display supports are to blit a bitmap onto the display. There are no functions for drawing shapes like lines or circles. You would have to write your own code to render the shape as a bitmap then blit that bitmap to the display. This means the low level access to the display is not very useful and with very few exceptions you'd be better off just using TFT_eSPI or a similar high level library.

### ESP-IDF

The Espressif ESP32 libraries do provide low level display functions for a few types of display, and this includes the ST7789/ILI9341 display used in the CYD. This sketch demonstrates how to use these functions.

### ILI9341-Driver

This sketch demonstrates how to control the display directly by sending control codes over the SPI bus. This is about as low level as you can get.