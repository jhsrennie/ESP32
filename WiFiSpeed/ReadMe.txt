WiFiSpeed
---------
This is a sketch to test the WiFi speed on an ESP32.

The sketch configures the ESP32 to act as an HTTP server. When a client connects it sends back 8MB of random data and times how long it takes to get the average speed. A convenient way to get the speed is to use curl on your PC:

curl http://1.2.3.4/ -o foo.txt

where you need to replace 1.2.3.4 by the IP address of the ESP32.

Line 17 of WiFiSpeed.ino sets the buffer size i.e. the number of bytes passed to client.write(). Larger buffers have a lower overhead and will give a faster transmission rate with lower CPU usage. The figures below test the effect of buffer size.

To see if the speed is CPU limited I added code that monitors the CPU usage. Just ignore this if you're not interested. Anyhow the CPU usage approaches 100% so it is CPU limited at least to some extent.

I ran the sketch on the original ESP32 and on a C3 with the results below. These were with signal strengths between -15 and -45dB. In this range the speed seems largely independent of the signal strength.

It's interesting to see that the ESP32 obviously uses both cores since the CPU usage exceeds 100%. The (single core) C3 only reaches approximately 90% so it is not completely CPU limited though the CPU is very high. The 240MHz ESP32 gives higher transmission speeds than the 160MHz C3, but not much higher.

ESP32 (original)
Buf size  Speed (KB/s)  CPU
  16384       1338      88%
   8192       1176      90%
   4096       1031      91%
   2048        809      93%
   1024        711     102%
    512        594     115%
    256        405     119%

ESP32 C3
Buf size  Speed (KB/s)  CPU
  16384       1236      75%
   8192       1079      77%
   4096        955      80%
   2048        696      81%
   1024        617      85%
    512        499      89%
    256        377      93%
