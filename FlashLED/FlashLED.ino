//----------------------------------------------------------------------
// Use the Arduino pin functions to flash the on board LED
// Different dev boards have the on board LED attached to different
// pins, and they even differ in whether HIGH turns on the LED or turns
// it off. We use conditional compilation to set the appropriate values
// for the board we are using
//----------------------------------------------------------------------

// The original ESP32 has the LED on pin 2 and HIGH turns the LED on
// while LOW turns it off.
#if CONFIG_IDF_TARGET_ESP32
#define LED_BUILTIN 2
#define LED_ON  HIGH
#define LED_OFF LOW

// The ESP32 C3 has the LED on pin 8 and HIGH turns the LED off while
// LOW turns it on.
#elif CONFIG_IDF_TARGET_ESP32C3
#define LED_BUILTIN 8
#define LED_ON  LOW
#define LED_OFF HIGH

// Warn if we have an unknown board
#else
#error Target CONFIG_IDF_TARGET is not supported
#endif

void setup() {
  // Configure the serial connection
  Serial.begin(115200);
  delay(1000);
  Serial.printf("FlashLED started on pin %d\n", LED_BUILTIN);

  // Set up the built in LED pin
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  Serial.println("Flash!");
  // On for 0.5 secs then off for 1.5 seca
  digitalWrite(LED_BUILTIN, LED_ON);
  delay(500);
  digitalWrite(LED_BUILTIN, LED_OFF);
  delay(1500);
}