### Introduction

The ESP32-2432S028R, better known as the Cheap Yellow Display has become a hobbyist favourite because it has a touchscreen display built in and ... well ... it's cheap! This directory has a collection of programs I have used for experimenting with a CYD.

### Graphics libraries

For graphics I usually use the TFT_eSPI library as it's available from the Arduino IDE library manager and it's easy to use. To use it install "TFT_eSPI by Bodmer" using the library manager then copy the file `User_Setup.h` into the `libraries\TFT_eSPI` directory replacing the file that is already there. Configuring the `User_Setup.h` file is the hardest part of getting TFT_eSPI working and the version I've included in this repository works for me.

Some of the sketches use the Arduino GFX library instead. The Arduino library is far easier to get working as it doesn't require navigating the (complicated!) `User_Setup.h` file.

### Arduino GFX bug

To use the Arduino GFX library you need the Adafruit ILI9341 library to drive the CYD's controller, however there seems to be issue with this on the CYD. To get it to work I had to edit the file `libraries\Adafruit_ILI9341\Adafruit_ILI9341.h` and change the screen size settings. At about line 45 (in v1.6.3 - it may change in later versions) you'll find the lines:

```
#define ILI9341_TFTWIDTH 240  ///< ILI9341 max TFT width
#define ILI9341_TFTHEIGHT 320 ///< ILI9341 max TFT height
```

I had to change these to:

```
#define ILI9341_TFTWIDTH 320  ///< ILI9341 max TFT width
#define ILI9341_TFTHEIGHT 240 ///< ILI9341 max TFT height
```

Unless you do this the display is corrupted. I tried reporting this to Adafruit but their view is they only support the Adafruit ILI9341 boards not the CYD. Oh well. Just remember you will need to redo this edit whenever a new version of the ILI9341 library is installed.

### Sketches

Adafruit  
This is example code to show how to use the Arduino_GFX library. The Arduino library is much easier to configure than TFT_eSPI and it's about the same size. It's more basic than TFT_eSPI but works fine for simple apps.

Backlight  
This is code to show how to adjust the intensity of the backlight using LEDC.

Blink  
This is example code to show how to control the on board LED.

Clock  
This is a sketch to make the CYD work as a digital clock using SNTP to get the time from the Internet. It requires an AHT20+BMP280 sensor to get the temperature, humidity and pressure, but it will work fine without it. You can choose either the TFT_eSPI or Arduino GFX graphics libraries.

Console  
This is a class using to implement a simple scrolling console on the CYD.

FontMetricsGFX  
This is a sketch to measure the sizes in pixels that the Arduino GFX free fonts are rendered on the CYD.

LDR  
This is example code to use the on board light dependent resistor.

LVGL  
This is example code to show how to use the LVGL library.

TFT_eSPI  
This is example code to show how to use the TFT_eSPI library.

Touch  
This is example code to show how to use the touch screen.
