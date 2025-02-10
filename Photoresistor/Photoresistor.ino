#define LED_BUILTIN 2
#define ADC_PIN 4

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(1000);
  Serial.println("Photoresistor started");

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // Read the photoresistor voltage
  int adcval = analogRead(ADC_PIN);
  float v = 3.3*adcval/4096;
  Serial.printf("ADC = %d, voltage = %f\n", adcval, v);

  // Set the on board LED
  if (v > 2)
    digitalWrite(LED_BUILTIN, LOW);
  else
    digitalWrite(LED_BUILTIN, HIGH);

  // Wait for a second
  delay(1000);
}