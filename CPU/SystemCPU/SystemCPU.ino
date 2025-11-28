//----------------------------------------------------------------------
// SystemCPU
// ---------
// Sketch to show how to monitor the system CPU usage
//----------------------------------------------------------------------
#include "SysCPUMon.h"

//----------------------------------------------------------------------
// setup
//----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  // Start the CPU monitoring
  StartSystemCPUMon();

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
  StopSystemCPUMon();
}

//----------------------------------------------------------------------
// loop
// This will never be called since setup() never returns
//----------------------------------------------------------------------
void loop() {
}