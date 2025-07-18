// Demonstrate running tasks on both cores
// For more details see:
// https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-reference/system/freertos.html

// Mutexs are done using semaphores:
// SemaphoreHandle_t baton;
// xSemaphoreTake(baton, portMAX_DELAY); // to acquire a semaphore
// xSemaphoreGive(baton); // to release the baton again


TaskHandle_t Task1;

void codeForTask1(void* Parameter)
{
  for (;;) {
    Serial.printf("Task1 running on core %d\n", xPortGetCoreID());
    delay(1000);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("FlashLED started");

  // Set up the built in LED pin
  pinMode(2, OUTPUT);

  // Create the parallel task
  xTaskCreatePinnedToCore(
    codeForTask1, // task function
    "Task1",      // task name
    1000,         // task stack size
    NULL,         // task parameter
    1,            // task priority
    &Task1,       // task handle
    0             // Core
  );
}

void loop() {
  Serial.printf("BlinkLED running on core %d\n", xPortGetCoreID());
  digitalWrite(2, HIGH);
  delay(500);
  digitalWrite(2, LOW);
  delay(500);
}
