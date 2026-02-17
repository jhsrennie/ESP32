GraphicsLibTest
===============
This is adapted from the speed test sketch provided as an example in the TFT_eSPI library, which was itself adapted from the Adafruit speed test sketch. I have modified the code so it can be compiled with the three common ESP32 graphics libraries:  
- TFT_eSPI
- LovyanGFX
- ArduinoGFX

I have included an Excel spreadsheet comparing the results from the three libraries. LovyanGFX is slightly faster than TFT_eSPI though the difference is not huge. ArduinoGFX is about 1.5 times slower than the other two at most things, but for some reason it is 4 times slower at drawing lines.