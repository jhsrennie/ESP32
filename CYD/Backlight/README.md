### Backlight

This is a sketch to show how to adjust the backlight brightness.

The backlight works like an LED on GPIO21, so set this pin HIGH to turn it on and LOW to turn it off. To get intermediate intensities use PWM and vary the duty cycle to vary the intensity. This sketch shows how to use the LEDC functions to do this.

The sketch reads commands you type into the serial monitor. Type a number from 0 to 100 to set the brightness or type `fadeup` or `fadedown` to fade the intensity up or down.