ESPWaveGen
-----------
This is a sketch to make the ESP32 work as a signal generator.

The signal generator is controlled through a web page. The sketch creates a wifi network with the SSID "ESPScope". Connect your wifi to this network then open:

http://192.168.4.1/

This will open a web page with controls to change the frquency and waveform type.

The output is on both DAC channels so pins 25 and 26.

The frequency will go up to 200KHz but the waveform quality is very poor at high frequencies due to the limited DAC conversion rate of the ESP32. However the wave quality is pretty good at audio frequencies.