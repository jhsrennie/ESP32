# ESP32

This is a collection of ESP32 sketches that I written. Mostly they were just written for fun rather than to actually be useful. However they show how to code lots of useful features.

They were all written on a WROOM but they should run on most ESP32s. Note however that some, like the C3, do not have a DAC built in so the sketches that use DACs won't work. All code uses the Arduino interface not the IDF.

ADC-DMA  
Code to test continuous A to D input. Also demonstrates use of the cosine generator.

C3-Temp  
Demonstrate use of the C3 on chip temperature sensor

Cosgen  
experiment with cosine generator.

CPU/SystemCPU  
Demonstrate how to monitor system CPU usage

CPU/TaskCPU  
Demonstrate how to monitor CPU usage for a single task

CYD  
Collection of sketches for the "Cheap Yellow Display"

DAC  
Very simple sketch using the Arduino functions to do analogue output and input.

DeepSleep  
Test using the ESP32 deep sleep function

DeepSleepGPIO  
Test using the ESP32 deep sleep function with wake by a GPIO pin

DeepSleepGPIOC3  
Test using the ESP32 deep sleep function with wake by a GPIO pin - C3 version

ESP-NOW  
Demonstrate communication between ESP32s using the ESP-NOW protocol.

ESPScope  
Oscilloscope sketch. Displays output through a web page.

ESPWaveGen  
Fully features signal generator.

FlashLED  
Code to flash an LED using the Arduino pin functions.

LedcGen  
Test the ledc functions for generating a square wave with user selectable frequency and duty cycle.

libraries/RHSSock  
BSD sockets class.

libraries/RHSWiFi  
WiFi class.

libraries/SoftAP  
Code to implement a WiFi access point.

MQTT  
Test MQTT using a third party class

MQTT_IDF  
Test MQTT using the IDF funtions

NTPClient  
This demonstrates how to set the time using the ESP-IDF SNTP functions

NTPServer  
This extends the NTPClient sketch to add an NTP server as well

Photoresistor  
Test use of a photoresistor as input

Server  
Implements a simple telnet server using Arduino funtions and also using BSD sockets.

SieveBenchmark  
Speed test sketch

SocketsAPI  
Test using BSD sockets for TCP/IP connections.

TestI2C  
Very simple test of I2C functions.

TestRHSSock  
Test the CRHSSockets class from libraries/RHSSock

TestRHSWiFi  
Test the CRHSWiFi class from libraries/RHSWiFi

TwoCore  
Test running tasks on both cores of the Xtensa CPU.

VL53L0X  
Test using a VL53L0X lidar sensor

WaveGen  
Test using continuous D to A conversion. Generates waveforms with various shapes.

WebServer  
Test code to demonstrate how to write a simple HTTP server using the WebServer class.

WebSock  
Test code to demonstrate how to use web sockets.

WiFiEvents  
This demonstrates how to use a wifi event handle with the Arduino IDE.

WiFiSpeed  
Test the wifi speed

W5500SpeedTest  
More advanced sketch that configures a W5500 and runs a web server on it.

W55500Test  
Sketch to show how to configure a W5500 ethernet module.
