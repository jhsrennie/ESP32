ESP-NOW
-------
This was written to test the ESP-NOW communication protocol.

You need to fill in the Peers array with the MAC addresses of all the boards you want to communicate. The sketch takes a string you type in at the serial monitor and it sends the string to all the addresses in the Peers array (apart from its own address). I wrote the sketch this way so you can run the same sketch on all the boards and they will all send data to each other.

The sketch will print confirmation that a string has been sent to the serial monitor, and whenever it receives a string it will print what it received to the serial monitor.

The ESP32 does not need to be connected to a wireless network, but it needs the wifi to be started. The sketch uses IDF functions to configure and start the network.