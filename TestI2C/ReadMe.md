# TestI2C

This is a very basic sketch to test using I2C. All it does is scan all the valid I2C addresses to see what devices are attached.

This example uses GPIO 22 for the SDA connection and GPIO 27 for the SCL connection, so you need to wire pin 22 on your ESP32 to SDA on your device and pin 27 to SCL.