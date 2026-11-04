# Arduino_GFX

This is as simple a text of the display as you can get. It uses Arduino_GFX so no setup is required except to install the Arduino_GFX and Arduino_ILI9341 libraries. Having done that the sketch should just work.

On my CYD the blue and red colours are reversed so:

    tft.fillScreen(ILI9341_RED);

actually fills the screen with blue, and vice versa. There is probably a setting somewhere in `Adafruit_ILI9341.h` to fix this but I haven't found it yet.