//----------------------------------------------------------------------
// TaskCPU
// -------
// Sketch to show how to monitor CPU usage by a task
//----------------------------------------------------------------------
#include "TaskCPUMon.h"

//----------------------------------------------------------------------
// setup
//----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  // Get this task handle. In the Arduino IDE the main task is called "loopTask".
  TaskHandle_t h = xTaskGetHandle("loopTask");
  if (!h) {
    Serial.println("fatal error: cannot get task handle");
    vTaskSuspend(NULL);
  }
  // Start the CPU monitoring
  StartTaskCPUMon(h);

  // Sit in a loop alternating between quiet and busy
  while (true) {
    // Do nothing for 10 secs
    Serial.println("CPU should be low now");
    delay(10000);

    // Then thrash the CPU for 10 secs
    Serial.println("CPU should be high now");
    int t = millis() + 10000;
    while (millis() < t) {
      // Some random calculation to use up CPU
      double x = sin((rand() % 628)/100.0);
    }
  }

  // Stop the CPU monitoring
  // We never actually get here, but this is how you'd stop the cpu
  // monitoring if we did.
  StopTaskCPUMon();
}

//----------------------------------------------------------------------
// loop
// This will never be called since setup() never returns
//----------------------------------------------------------------------
void loop() {
}