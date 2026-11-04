//**********************************************************************
// GC9A01
// ======
// Example sketch to show how to use the GC9A01 display on an ESP32
// using the Lovyan library.
//**********************************************************************
#include <LovyanGFX.hpp>
// This file contains the settings for the ILI9341 controller
#include "LovyanGFX_GC9A01.h"

LGFX tft;

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup() {
  // Initialise the serial monitor and wait for it to complete
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting GC9A01 with the Lovyan library");

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