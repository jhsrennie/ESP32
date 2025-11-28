//----------------------------------------------------------------------
// SysCPUMon.cpp
// -------------
// Code to implement system CPU usage monitoring.
// This logs the total CPU usage to the serial monitor.
// It works by calculating the % CPU used by the idle task(s) and
// subtracting that from 100%.
// The usage is up to 100% for each CPU so for the dual CPU ESP32s this
// will report up to 200% CPU.
// The monitoring runs in a separate task. We create the task to start
// the monitoring and delete the task to stop it.
//----------------------------------------------------------------------
#include <Arduino.h>
#include "SysCPUMon.h"

// Handle for the monitoring task
TaskHandle_t hCPUUsageTask = NULL;

//----------------------------------------------------------------------
// CPUUsageTask
// ------------
// The task monitors the CPU usage
//----------------------------------------------------------------------
static void CPUUsageTask(void* Parameter)
{
  int num_cpus;
  unsigned long runtime;
  unsigned long last_runtime[2] = {0};
  TaskHandle_t htask[2];

  // Print the task stats summary
  // This is not essential - it;s just for fun
  char* buf = (char*) malloc(1000);
  Serial.println("Task list:");
  vTaskGetRunTimeStats(buf);
  Serial.print(buf);
  free(buf);

  // We need the IDLE task so we can follow its CPU usage
  // On single core ESP32s this is just called "IDLE"
  htask[0] = xTaskGetHandle("IDLE");
  if (htask[0]) {
    num_cpus = 1;
  }
  // On dual core ESP32s there are two tasks "IDLE0" and "IDLE1"
  else {
    htask[0] = xTaskGetHandle("IDLE0");
    htask[1] = xTaskGetHandle("IDLE1");
    num_cpus = 2;
    // If we cannot find the idle task give up
    if (!htask[0] || !htask[1]) {
      Serial.println("CPUUsage: cannot find IDLE task");
      vTaskSuspend(NULL);
    }
  }

  // Loop checking the CPU usage by the IDLE task(s)
  // The way this works is we get the ticks used by the idle task, wait
  // a second then get the ticks again. One tick is one microsecond to
  // divide the ticks used in the last second by 1000000 to get the
  // fraction of the last second that the idle task ran.
  // For the dual core CPUs we do this for both the idle tasks.
  while (true) {
    TaskStatus_t xTaskDetails;

    // Get the runtime for the first idle task
    vTaskGetInfo(htask[0], &xTaskDetails, pdTRUE, eInvalid);
    runtime = xTaskDetails.ulRunTimeCounter - last_runtime[0];
    last_runtime[0] = xTaskDetails.ulRunTimeCounter;
    // If there is a second CPU get its runtime as well and add it to
    // the first runtime to get the total for both tasks
    if (num_cpus == 2) {
      vTaskGetInfo(htask[1], &xTaskDetails, pdTRUE, eInvalid);
      runtime += xTaskDetails.ulRunTimeCounter - last_runtime[1];
      last_runtime[1] = xTaskDetails.ulRunTimeCounter;
    }

    // Calculate the run time used
    float cpu_usage = num_cpus*100.0 - runtime/10000.0;
    Serial.printf("%.1f\n", cpu_usage);

    // Pause for a second
    delay(1000);
  }
}

//----------------------------------------------------------------------
// StartCPUUsageTask
// -----------------
// Start the CPU usage monitoring task
//----------------------------------------------------------------------
void StartSystemCPUMon() {
  // If the task is currently running kill it
  if (hCPUUsageTask)
    StopSystemCPUMon();

  // Create the task to monitor CPU
  xTaskCreate(CPUUsageTask, "CPUUsageTask", 3000, NULL, 1, &hCPUUsageTask);
  Serial.println("CPU usage task created");
}

//----------------------------------------------------------------------
// StopCPUUsageTask
// ----------------
// Stop the CPU usage monitoring task
//----------------------------------------------------------------------
void StopSystemCPUMon() {
  // Check if the task is running
  if (!hCPUUsageTask)
    return;

  // Stop the task
  vTaskDelete(hCPUUsageTask);
  hCPUUsageTask = NULL;
  Serial.println("CPU usage task deleted");
}