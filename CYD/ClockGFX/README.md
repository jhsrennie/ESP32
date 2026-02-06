### Clock

This is a sketch to make the CYD act as a digital clock.

The clock gets its time from the Internet using SNTP. My code also uses an AHT20+BMP280 sensor to show the temperature, humidity and pressure. The sensor is connected to the CN1 port on the CYD and is uses GPIO 22 for SDA and GPIO 27 for SCL. If either sensor is not present the code still works fine but will not display the missing data.

The clock fades the display after 20s to save power. Touching the screen will bring the brightness back up to maximum.

This uses the Adafruit GFX library since it needs only basic graphics and it's easier to configure than TFT_eSPI. The seven segment clock font was generated from https://oleddisplay.squix.ch/#/home