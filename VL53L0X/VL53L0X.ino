#include <Adafruit_VL53L0X.h>

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup() {
  Serial.begin(115200);
  while (! Serial)
    delay(100);

  // Initialise the VL53L0X
  Serial.println("Initialising VL53L0X ...");
  if (!lox.begin()) {
    Serial.println(F("Failed to initialise VL53L0X"));
    vTaskSuspend(NULL);
  }
  Serial.println("Initialised");
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    Serial.print("Distance (mm): "); Serial.println(measure.RangeMilliMeter);
  } else {
    Serial.println(" out of range ");
  }

  delay(1000);
}
