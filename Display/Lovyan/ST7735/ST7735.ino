//**********************************************************************
// ST7735
// ======
// Example sketch to show how to use the ST7735 display on an ESP32
// using the Lovyan library.
//**********************************************************************
#include <LovyanGFX.hpp>
// This file contains the settings for the ST7735 controller
#include "LovyanGFX_ST7735.h"

LGFX tft;

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup() {
  // Initialise the serial monitor and wait for it to complete
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting ST7735 with the Lovyan library");

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