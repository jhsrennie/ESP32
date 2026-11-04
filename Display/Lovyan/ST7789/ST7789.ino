//**********************************************************************
// ST7789
// ======
// Example sketch to show how to use the ST7789 display on an ESP32
// using the Lovyan library.
//**********************************************************************
#include <LovyanGFX.hpp>
// This file contains the settings for the ST7789 controller
#include "LovyanGFX_ST7789.h"

LGFX tft;

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup() {
  // Initialise the serial monitor and wait for it to complete
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting ST7789 with the Lovyan library");

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
  delay(1000);

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setFont(&fonts::Font4);
  tft.drawString("Lovyan ST7789", 30, 120);
  delay(5000);
}