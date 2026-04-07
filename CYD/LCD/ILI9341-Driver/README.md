# ILI9341-Driver

This shows how the ILI9341 controller in the CYD can be controlled without using any external libraries. The two `esp_lcd_ili9341` files contain code that drives the ILI9341 directly by sending it commands over the SPI bus, and `ILI9341-Driver.ino` shows how you would use this in your sketch.

This is really only for curiousity since there is little point in doing this. The only operation the ILI9341 supports is blitting a bitmap onto some location on the screen so there isn't much you could do with it.