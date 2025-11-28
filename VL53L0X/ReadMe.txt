VL53L0X
-------
This is a sketch to test the VL53L0X lidar sensor.

Connect the sensor to the I2C pins. Typically the SCL pin connects to GPIO 22 and the SDA pin connects to GPIO 21. You can take power and round from the ESP32 as the VL53L0X draws hardly any power.

You will need to install the AdaFruit library for the VL53L0X.

The sketch writes the measured distance to the serial monitor.