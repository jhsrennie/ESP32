# SSD1306

This demonstrates how to use an SSD1306 I2C display with the Arduino_GFX library.

To use this you need to install the _Adafruit SSD1306 by Adafruit_ library from the Library Manager.

This display uses an I2C interface so you need to connect the SDA and SCL pins on the display to your ESP32. The example assumes you have connected the pins to:
```
Display    ESP32
SDA       GPIO 21
SCL       GPIO 22
```
These are the default I2C pins used by the Arduino library for the ESP32 but you could use any two pins as I2C will work on any pins. You may need to choose different pins on some supermini boards that do not expose GPIOs 21 and 22.