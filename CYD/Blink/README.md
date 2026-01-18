### Blink

This shows how to blink the three coloured LEDs on the CYD.

Unlike the S3 and C6 the CYD does not use a WS2812 controller. Instead it has three separate LEDS with their own GPIO pins:
Red   GPIO  4
Green GPIO 16
Blue  GPIO 17

Note that the LEDs are connected to the positive rail so you need to set the GPIOs LOW to illuminate the LED.