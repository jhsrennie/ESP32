### Clock

This is a sketch to make the CYD act as a digital clock.

The clock gets its time from the Internet using SNTP. My code also uses an AHT20+BMP280 sensor to show the temperature, humidity and pressure. The sensor is connected to the CN1 port on the CYD. The clock fades the display after 20s to save power. Touching the screen will bring the brightness back up to maximum.