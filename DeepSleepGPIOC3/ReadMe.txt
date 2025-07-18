DeepSleepGPIOC3
---------------
This shows how to configure an ESP32 C3 for deep sleep with a wake signal from a GPIO pin.

The C3 only allows wake from GPIO on the pins powered by VDD3P3_RTC, which is normally pins 0 to 5. Trying to call esp_deep_sleep_enable_gpio_wakeup() on any other pins will return an error.

Connect a switch to pulse pin 2 high then run the sketch and pressing the switch will wake the ESP32.