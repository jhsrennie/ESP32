//----------------------------------------------------------------------
// Blink
// =====
// Demonstrate how to flash the LEDs on the CYD
//----------------------------------------------------------------------

#define PIN_RED    4
#define PIN_GREEN 16
#define PIN_BLUE  17
#define LED_ON   LOW
#define LED_OFF HIGH

void setup() {
  // Configure the serial connection
  Serial.begin(115200);
  delay(1000);
  Serial.println("Blink started");

  pinMode(PIN_RED,   OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE,  OUTPUT);

  digitalWrite(PIN_RED,   LED_OFF);
  digitalWrite(PIN_GREEN, LED_OFF);
  digitalWrite(PIN_BLUE,  LED_OFF);
}

void loop() {
  Serial.println("Flash!");

  digitalWrite(PIN_RED, LED_ON);
  delay(1000);

  digitalWrite(PIN_GREEN, LED_ON);
  delay(1000);

  digitalWrite(PIN_RED, LED_OFF);
  delay(1000);

  digitalWrite(PIN_BLUE, LED_ON);
  delay(1000);

  digitalWrite(PIN_GREEN, LED_OFF);
  delay(1000);

  digitalWrite(PIN_RED, LED_ON);
  delay(1000);

  digitalWrite(PIN_BLUE, LED_OFF);
}