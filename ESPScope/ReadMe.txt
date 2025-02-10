ESPScope
--------
This is a sketch to make the ESP32 work as an oscilloscope.

The output is displayed through a web page. The sketch creates a wifi network with the SSID "ESPScope". Connect your wifi to this network then open:

http://192.168.4.1/

This will open a web page that shows the output from the scope and has controls to change the time base.

The input is on ADC 1 channel 0, which is usually pin 36. There is a 1KHz calibration signal on pin 25, so for testing you can connect pin 25 to pin 36.

It is not a very useful oscilloscope because the ESP32 ADC sampling rate is too low to be useful much above audio frequencies. However it does work and it shows how to code a number of features including:
- how to create a wireless access point
- a web server
- continuous analogue to digital conversion
- the built in cosine generator
