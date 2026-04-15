AD8232
======
This is a sketch to record ECG data using the AD8232 cardiac monitor.

This written for an S3 supermini and uses pins:
 8 - output
 9 - L0-
10 - L0+

It will work on any ESP32 but you need to change the pin defines in Data.cpp to match the pins used on your board.

The kit I got from AliExpress has yellow, red and green wires. Connect these as follows:
yellow - left arm
red    - right arm
green  - right leg

The AD8232 board works but I found the data was very noisy and you had to thoroughly relax to get anything resembling the usual ECG trace.

The sketch connects to a wi-fi network and displays the data using a web server. You will need to change the SSID and password defines in AD8232.ino to match your network. To see the data use the URL http://<IP address>/ where the IP address is the address of the ESP32. The address is reported in the serial monitor.