//----------------------------------------------------------------------
// LDR test
// --------
// The LDR is on pin 34, ADC1 CH6
// The LDR is very sensitive and will read 0 in even dim conditions. In
// complete darkness it reads about 1200.
// So it isn't very useful.
//----------------------------------------------------------------------
#define GPIO_LDR 34

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("LDR test");
  analogSetAttenuation(ADC_0db);
}

void loop() {
  uint16_t ldr = analogRead(GPIO_LDR);
  Serial.printf("LDR = %d\n", (int) ldr);
  delay(1000);
}