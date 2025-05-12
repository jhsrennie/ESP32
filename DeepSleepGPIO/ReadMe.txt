DeepSleepGPIO
-------------
This shows how to configure an ESP32 for deep sleep with a wake signal from a GPIO pin.

Connect a switch to pulse pin 2 high then run the sketch and pressing the switch will wake the ESP32.

This does not work on the C3 as it seems the API changed between the original ESP32 and the C3. See the DeepSleepGPIOC3 sketch for code that works on the C3.