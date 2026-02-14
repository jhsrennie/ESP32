### Clock

This is a sketch to make the CYD act as a digital clock.

This has been written so it can use either the TFT_eSPI library or the ArduinoGFX library. They are similar enough that only a few conditional blocks are needed. The 48pt seven segment clock font was generated from https://oleddisplay.squix.ch/#/home

The clock gets its time from the Internet using SNTP. My code also uses an AHT20+BMP280 sensor to show the temperature, humidity and pressure. The sensor is connected to the CN1 port on the CYD and is uses GPIO 22 for SDA and GPIO 27 for SCL. If either sensor is not present the code still works fine but will not display the missing data.

The clock fades the display after 20s to save power. Touching the screen will bring the brightness back up to maximum.

You can use either TFT_eSPI or the Adafruit GFX library as you prefer. If you're using the Adafruit library see the note on the screen issue with the CYD in the CYD/README.md directory.