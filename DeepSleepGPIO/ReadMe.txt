DeepSleepGPIO
-------------
This shows how to configure an ESP32 for deep sleep with a wake signal from a GPIO pin.

Connect a switch to pulse pin 2 high then run the sketch and pressing the switch will wake the ESP32.

This will not work on varients like the C3 that do not support RTC GPIO.