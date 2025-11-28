#include <Wire.h>
 
void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial.println("\nI2C Scanner");
}

bool ScanI2C(byte Address) {
  Wire.beginTransmission(Address);
  byte error = Wire.endTransmission();
  return (error == 0);
}
 
void loop() {
  if (ScanI2C(0x29))
    Serial.println("Found device at address 0x29");
  else
    Serial.println("No device found at address 0x29");
  delay(5000);          
}