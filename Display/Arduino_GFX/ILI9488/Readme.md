# ILI9488

I do not recommend using the ILI9488 with the Arduino_GFX library. Use TFT_eSPI or Lovyan instead.

The Arduino_GFX library uses 16 bit colour while the ILI9488 uses 24 bit colour. There are libraries that will allow the ILI9488 to work with Arduino_GFX, e.g. [Jaret Burkett's library](https://github.com/jaretburkett/ILI9488), but performance will suffer due to the 24 to 16 bit conversions.