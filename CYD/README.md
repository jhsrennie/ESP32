The ESP32-2432S028R, better known as the Cheap Yellow Display has become a hobbyist favourite because it has a touchscreen display built in and ... well ... it's cheap! This directory has a collection of programs I have used for experimenting with a CYD.

For graphics I usually use the TFT_eSPI library as it's available from the Arduino IDE library manager and it's easy to use. To use it install "TFT_eSPI by Bodmer" using the library manager then copy the file `User_Setup.h` into the `libraries\TFT_eSPI` directory replacing the file that is already there. Configuring the `User_Setup.h` file is the hardest part of getting TFT_eSPI working and the version I've included in this repository works for me.

Adafruit  
This is example code to show how to use the Arduino_GFX library. The Arduino library is much easier to configure than TFT_eSPI and it's about the same size. It's more basic than TFT_eSPI but works fine for simple apps.

Backlight  
This is code to show how to adjust the intensity of the backlight using LEDC

Blink  
This is example code to show how to control the on board LED

Clock  
This is a sketch to make the CYD work as a digital clock using SNTP to get the time from the Internet. It requires an AHT20+BMP280 sensor to get the temperature, humidity and pressure, but it will work fine without it. This version uses FTF_eSPI for the graphics.

ClockGFX
This is the clock app using the ArduinoGFX library instead of TFT_eSPI.

LDR  
This is example code to use the on board light dependent resistor

LVGL  
This is example code to show how to use the LVGL library

TFT_eSPI  
This is example code to show how to use the TFT_eSPI library

Touch  
This is example code to show how to use the touch screen
