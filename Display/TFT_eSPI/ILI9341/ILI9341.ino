//**********************************************************************
// ILI9341
// =======
// Example sketch to show how to use the ILI9341 display on an ESP32
// using the TFT_eSPI library.
//**********************************************************************
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup() {
  // Initialise the serial monitor and wait for it to complete
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting ILI9341 with the TFT_eSPI library");

  // Init display
  tft.init();
}

//----------------------------------------------------------------------
// loop
// ----
//----------------------------------------------------------------------
void loop() {
  // As a test cycle through various screen colours
  tft.fillScreen(TFT_RED);
  delay(1000);
  tft.fillScreen(TFT_GREEN);
  delay(1000);
  tft.fillScreen(TFT_BLUE);
  delay(5000);
}