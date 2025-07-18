Server
------
This is a simple network server.

There are two versions of the server. The file Server.wifiserver uses the Arduino WiFiServer class for the network communication while the file Server.sockets uses the BSD sockets API. To run the sketch copy whichever version you want to Server.ino then compile it.

The sketch creates a telnet server listening on port 23. Connect to it using whatever telnet client you prefer. You will need to edit the code to set your own wifi SSID and password. Once connected you can type the commands:
  on - turn on the on board LED
  off - turn off the on board LED
  exit - close the connection

Note that the Arduino WiFi class may use the on board LED to show the connection status. If your dev board does this you will need to use a different GPIO pin and connect an external LED.

The code uses the RTOS task functions to create separate tasks to handle the connection and control the LED. There's no special reason to do this. I was just curious to see how tasks work on the ESP32.