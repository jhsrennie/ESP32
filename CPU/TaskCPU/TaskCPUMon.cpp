//----------------------------------------------------------------------
// TaskCPUMon.cpp
// --------------
// Code to implement task CPU usage monitoring.
// This logs the CPU used by the task to the serial monitor.
// The monitoring runs in a separate task. We create the task to start
// the monitoring and delete the task to stop it.
//----------------------------------------------------------------------
#include <Arduino.h>
#include "TaskCPUMon.h"

// Handle for the monitoring task
TaskHandle_t hTaskCPUMon = NULL;

//----------------------------------------------------------------------
// TaskCPUMonTask
// --------------
// The task monitors the CPU usage for the supplied task handle
//----------------------------------------------------------------------
static void TaskCPUMonTask(void* Parameter)
{
  unsigned long runtime;
  unsigned long last_runtime = 0;
  TaskHandle_t htask = (TaskHandle_t) Parameter;

  // Print the task stats summary
  // This is not essential - it's just for fun
  char* buf = (char*) malloc(1000);
  Serial.println("Task list:");
  vTaskGetRunTimeStats(buf);
  Serial.print(buf);
  free(buf);

  // Loop checking the CPU usage by the task
  // The way this works is we get the ticks used by the task, wait
  // a second then get the ticks again. One tick is one microsecond so
  // divide the ticks used in the last second by 1000000 to get the
  // fraction of the last second that the task ran.
  while (true) {
    TaskStatus_t xTaskDetails;

    // Get the runtime for the task
    vTaskGetInfo(htask, &xTaskDetails, pdTRUE, eInvalid);
    runtime = xTaskDetails.ulRunTimeCounter - last_runtime;
    last_runtime = xTaskDetails.ulRunTimeCounter;

    // Calculate the run time used
    float cpu_usage = runtime/10000.0;
    Serial.printf("%.1f\n", cpu_usage);

    // Pause for a second
    delay(1000);
  }
}

//----------------------------------------------------------------------
// StartTaskCPUMon
// ---------------
// Start the task CPU usage monitoring
//----------------------------------------------------------------------
void StartTaskCPUMon(TaskHandle_t hTask) {
  // If the task is currently running kill it
  if (hTaskCPUMon)
    StopTaskCPUMon();

  // Create the task to monitor CPU
  xTaskCreate(TaskCPUMonTask, "TaskCPUMonTask", 3000, (void*) hTask, 1, &hTaskCPUMon);
  Serial.println("Task CPU monitoring created");
}

//----------------------------------------------------------------------
// StopTaskCPUMon
// --------------
// Stop the CPU usage monitoring task
//----------------------------------------------------------------------
void StopTaskCPUMon() {
  // Check if the task is running
  if (!hTaskCPUMon)
    return;

  // Stop the task
  vTaskDelete(hTaskCPUMon);
  hTaskCPUMon = NULL;
  Serial.println("Task CPU monitoring stopped");
}