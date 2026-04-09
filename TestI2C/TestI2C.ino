#include <Wire.h>

#define SDA 22
#define SCL 27

void setup() {
  Wire.begin(SDA, SCL);
  Serial.begin(115200);
  Serial.println("I2C Scanner");
}

bool ScanI2C(byte Address) {
  Wire.beginTransmission(Address);
  byte error = Wire.endTransmission();
  return (error == 0);
}
 
void loop() {
  Serial.println("Scanning now ...");

  for (int id = 0x08; id <= 0x77; id++) {
    if (ScanI2C(id))
      Serial.printf("Found device at address 0x%02x\n", id);
  }

  Serial.println("Scan completed");

  delay(5000);          
}