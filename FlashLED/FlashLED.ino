//----------------------------------------------------------------------
// Use the Arduino pin functions to flash the on board LED
//----------------------------------------------------------------------
#define LED_BUILTIN 2

void setup() {
  Serial.begin(115200);
  while (!Serial)
    delay(1000);
  Serial.println("FlashLED started");

  // Set up the built in LED pin
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  Serial.println("Flash!");
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}