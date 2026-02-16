### Introduction

The ESP32-2432S028R, better known as the Cheap Yellow Display has become a hobbyist favourite because it has a touchscreen display built in and ... well ... it's cheap! This directory has a collection of programs I have used for experimenting with a CYD.

### Graphics libraries

TFT_eSPI  
There is a `User_Setup.h` file in the CYD directory. This needs to be copied into the `libraries\TFT_eSPI` directory.

LovyanGFX  
There is a `LovyanGFX_CYD_Settings.h` file in the CYD directory. This needs to be copied into your sketch directory and #included in your sketch.

ArduinoGFX  
You don't need to do anything special to use this.

The GraphicsLibTest sketch demonstrates how to use the three libraries. They are similar enough that you probably don't need to change your code in many cases.

### Arduino GFX problem

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

GraphicsLibTest  
This is a sketch to show how the three graphics libraries described above can be used in your code.

LDR  
This is example code to use the on board light dependent resistor.

LVGL  
This is example code to show how to use the LVGL library.

Touch  
This is example code to show how to use the touch screen.
