TaskCPU
-------
This is a sketch to show how to monitor the CPU usage by a single task on an ESP32.

To add monitoring to your sketch copy the two files TaskCPUMon.cpp and TaskCPUMon.h into the sketch folder, add:

#include "TaskCPUMon.h"

to you .ino file and then call StartTaskCPUMon() to start the monitoring and StopTaskCPUMon() to stop it. The task CPU usage will be printed to the serial monitor once a second.

To demonstrate the monitoring this sketch alternates between doing nothing and using 100% CPU.