MQTT_IDF
--------
This shows how to use the MQTT support built into the ESP32.

The sketch uses the free EMQX server and subscribes to the topic "rattus/test". You can type a string at the serial monitor and the code will publish it in the topic.

The ESP32 will respond to the messages "on" and "off" in the topic by turning the on board LED on or off.

You will need to edit the code to add your wifi SSID and password.