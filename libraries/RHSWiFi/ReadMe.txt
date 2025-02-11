RHSWifi
-------
This is my own version of a WiFi class for the ESP32.

The only advantage it has over the built in WiFi class is that on some development boards the built in wifi uses the on board LED to show the connection status, and that means you can use the built in LED for other things. My class does not tamper with the on board LED.

I suspect this will only be of interest if you are curious how the IDF wifi functions work.

See the TestRHSWifi sketch for how to use the class.