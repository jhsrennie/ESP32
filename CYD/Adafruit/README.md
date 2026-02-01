### Adafruit

This is the Adafruit library example from the [ESP32-Cheap-Yellow-Display repository](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) on GitHub. This is [the link to the code](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/Examples/AlternativeLibraries/Adafruit_ILI9341/1-Adafruit_ILI9341_Test/1-Adafruit_ILI9341_Test.ino).

The original code from the CYD repository used software SPI i.e. the line:

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST, TFT_MISO);

configures the library to use software SPI, and this is painfully slow. I replace this line with:

SPIClass tftSPI = SPIClass(HSPI);
Adafruit_ILI9341 tft = Adafruit_ILI9341(&tftSPI, TFT_DC, TFT_CS, TFT_RST);

to use hardware SPI and this is much faster.

To compile this you need to install the "Adafruit ILI9341 by Adafruit" library, but while this works on my board the orientation was not handled properly. I had to edit the file Adafruit_ILI9341.h in the library and change the lines:

```
#define ILI9341_TFTWIDTH 240  ///< ILI9341 max TFT width
#define ILI9341_TFTHEIGHT 320 ///< ILI9341 max TFT height
```

to:

```
#define ILI9341_TFTWIDTH 320  ///< ILI9341 max TFT width
#define ILI9341_TFTHEIGHT 240 ///< ILI9341 max TFT height
```

i.e. swap the width and height.