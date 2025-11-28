//----------------------------------------------------------------------
// Test simple use of the DAC and ADC
//----------------------------------------------------------------------
#define DAC_PIN 25
#define ADC_PIN 34

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(1000);
  Serial.println("DAC test started");

  pinMode(ADC_PIN, INPUT);
  pinMode(DAC_PIN, OUTPUT);
}

void loop() {
  for (uint8_t i = 0; i <= 16; i++) {
    // Write the value to the DAC
    uint8_t dacval = i == 16 ? 255 : i*16;
    Serial.printf("dacWrite(%d)\n", dacval);
    dacWrite(DAC_PIN, dacval);

    // Read the value back through the ADC
    int adcval = analogRead(ADC_PIN);
    Serial.printf("ADC = %d\n", adcval);

    delay(1000);
  }
}