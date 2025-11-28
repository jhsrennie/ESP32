SystemCPU
---------
This is a sketch to show how to monitor the system CPU usage on an ESP32.

To add monitoring to your sketch copy the two files SysCPUMon.cpp and SysCPUMon.h into the sketch folder, add:

#include "SysCPUMon.h"

to you .ino file and then call StartSystemCPUMon() to start the monitoring and StopSystemCPUMon() to stop it. The system CPU usage will be printed to the serial monitor once a second.

To demonstrate the monitoring this sketch alternates between doing nothing and using 100% CPU.