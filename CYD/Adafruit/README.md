### Adafruit

This is the Adafruit library example from the [ESP32-Cheap-Yellow-Display repository](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) on GitHub. This is [the link to the code](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/Examples/AlternativeLibraries/Adafruit_ILI9341/1-Adafruit_ILI9341_Test/1-Adafruit_ILI9341_Test.ino).

The program shows that the CYD can be used with the Adafruit GFX library, but it's very slow compared to TFT_eSPI. If for some reason you are determined not to use TFT_eSPI and want an alternative this probably is not it.

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