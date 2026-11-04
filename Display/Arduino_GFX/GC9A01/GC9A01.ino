//----------------------------------------------------------------------
// GC9A01
// ------
// Example sketch to show how to use the GC9A01 display on an ESP32
// using the Arduino GFX library.
//----------------------------------------------------------------------
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <SPI.h>

// Define the pins used
#define PIN_MOSI       2
#define PIN_MISO      -1 // The GC9A01 does not use MISO
#define PIN_SCK       15
#define PIN_DC         4
#define PIN_CS        16
#define PIN_RST       17

// Create the display object
// Adafruit_GC9A01A tft(&SPI, PIN_CS, PIN_DC, PIN_RST);
Adafruit_GC9A01A tft(PIN_CS, PIN_DC, PIN_RST);

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup(void) {
  // Initialise the serial monitor and wait for it to complete
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting GC9A01 with the Arduino_GFX library");

  // Configure the SPI bus to use our selected pins
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
  // SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);

  // Initialise the GC9A01
  tft.begin();

  Serial.println("Initialised");
}

//----------------------------------------------------------------------
// loop
// ----
//----------------------------------------------------------------------
void loop() {
  tft.fillScreen(GC9A01A_RED);
  delay(1000);
  tft.fillScreen(GC9A01A_GREEN);
  delay(1000);
  tft.fillScreen(GC9A01A_BLUE);
  delay(5000);
}