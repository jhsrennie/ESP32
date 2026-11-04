//**********************************************************************
// ST7789
// ======
// Example sketch to show how to use the ST7789 display on an ESP32
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
  Serial.println("Starting ST7735 with the TFT_eSPI library");

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
  tft.fillRect(0, 0, 80, 160, TFT_RED);
  delay(1000);
  tft.fillScreen(TFT_GREEN);
  delay(1000);
  tft.fillScreen(TFT_BLUE);
  delay(1000);

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextFont(4);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Hello World!", 120, 120);
  delay(5000);
}