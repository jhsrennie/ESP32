# ESP32

This is a collection of ESP32 sketches that I written. Mostly they were just written for fun rather than to actually be useful. However they show how to code lots of useful features.

They were all written on a WROOM but they should run on most ESP32s. Note however that some, like the C3, do not have a DAC built in so the sketches that use DACs won't work. All code uses the Arduino interface not the IDF.

ADC-DMA  
Code to test continuous A to D input. Also demonstrates use of the cosine generator.

Cosgen  
experiment with cosine generator.

DAC  
Very simple sketch using the Arduino functions to do analogue output and input.

DeepSleep  
Test using the ESP32 deep sleep function

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

MQTT  
Test MQTT using a third party class

MQTT_IDF  
Test MQTT using the IDF funtions

Photoresistor  
Test use of a photoresistor as input

Server  
Implements a simple telnet server using Arduino funtions and also using BSD sockets.

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
