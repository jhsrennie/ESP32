//----------------------------------------------------------------------
// FontMetricsGFX
// ==============
// Sketch to measure the sizes in pixels of ArduinoGFX fonts
//----------------------------------------------------------------------
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include "FontMetricsGFX.h"

// Test test
#define TEST_TEXT "bghjkpry"

// Screen size
#define SCREEN_WIDTH 320
#define SCREEN_DEPTH 240

// Colours
#define COL_BACK      ILI9341_BLACK
#define COL_FORE      ILI9341_WHITE

// Backlight
#define LED_PIN 21

// Create the global display object
#define TFT_CS   15 
#define TFT_DC    2
#define TFT_RST  -1

SPIClass tftSPI = SPIClass(HSPI);
Adafruit_ILI9341 tft = Adafruit_ILI9341(&tftSPI, TFT_DC, TFT_CS, TFT_RST);

void TestFont(int vOffset) {
    tft.fillScreen(COL_BACK);

  // Measure the text depth
  tft.setCursor(0, SCREEN_DEPTH/2);
  tft.printf("%s\n", TEST_TEXT);
  int depth = tft.getCursorY() - SCREEN_DEPTH/2;
  Serial.printf("Depth = %d pixels\n", depth);

  // And draw a rectangle behind the text
  tft.fillScreen(COL_BACK);

  int base = depth;
  tft.fillRect(0, base-depth+vOffset, SCREEN_WIDTH, depth, ILI9341_RED);
  tft.setCursor(0, base);
  tft.printf(TEST_TEXT);

  base = depth*2;
  tft.fillRect(0, base-depth+vOffset, SCREEN_WIDTH, depth, ILI9341_BLUE);
  tft.setCursor(0, base);
  tft.printf(TEST_TEXT);

  base = depth*3;
  tft.fillRect(0, base-depth+vOffset, SCREEN_WIDTH, depth, ILI9341_GREEN);
  tft.setCursor(0, base);
  tft.printf(TEST_TEXT);

  // And wait for user input
  Serial.println("Press return to continue");
  String s = Serial.readStringUntil(10);
}

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup() {
  Serial.setTimeout(1000*1000); // Long wait for user input
  Serial.begin(115200);
  delay(2000);

  // Turn the backlight on
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // Initialise the display
  tft.begin();
  tft.setRotation(0);
  tft.setTextColor(COL_FORE);

  // 9pt Sans
  Serial.println("Testing FreeSans9pt7b");
  tft.setFont(&FreeSans9pt7b);
  TestFont(VOFFSET_9PT);

  // 12pt Sans
  Serial.println("Testing FreeSans12pt7b");
  tft.setFont(&FreeSans12pt7b);
  TestFont(VOFFSET_12PT);

  // 18pt Sans
  Serial.println("Testing FreeSans18pt7b");
  tft.setFont(&FreeSans18pt7b);
  TestFont(VOFFSET_18PT);

  // 24pt Sans
  Serial.println("Testing FreeSans24pt7b");
  tft.setFont(&FreeSans24pt7b);
  TestFont(VOFFSET_24PT);

// Halt now the tests are done
  vTaskSuspend(NULL);
}

//----------------------------------------------------------------------
// loop
// ----
// Not used
//----------------------------------------------------------------------
void loop() {
}
