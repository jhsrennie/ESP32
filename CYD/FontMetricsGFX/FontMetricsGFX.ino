//----------------------------------------------------------------------
// FontMetricsGFX
// ==============
// Sketch to measure the sizes in pixels of ArduinoGFX fonts
// The Arduino fonts are rendered at 141 dpi
// LINE_DEPTH is the spacing between lines - ascent + descent + leading
// CHAR_DEPTH is the ascent + descent, but no leading
// DESCENDER is just the descent
//
// In the GFXglyph struct the yOffset is the number of pixels from the
// baseline to the top of the character so this is the font ascent.
// The descent = height - yOffset.
// The sketch goes through all the characters in the font and finds the
// maximum ascent and descent, and the size = ascent + descent. The line
// depth is greater than this as it includes leading space to make the
// text look less cramped.
//----------------------------------------------------------------------
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>

// Test test
#define TEST_TEXT "BbGgHhJjKkPpRrYy"

// Screen size
#define SCREEN_WIDTH 320
#define SCREEN_DEPTH 240

// Colours
#define COL_BACK ILI9341_BLACK
#define COL_FORE ILI9341_WHITE

// Backlight
#define LED_PIN 21

// Create the global display object
#define TFT_CS   15 
#define TFT_DC    2
#define TFT_RST  -1

SPIClass tftSPI = SPIClass(HSPI);
Adafruit_ILI9341 tft = Adafruit_ILI9341(&tftSPI, TFT_DC, TFT_CS, TFT_RST);

//----------------------------------------------------------------------
// GetGFXFontMetrics
// -----------------
// Function to examine a GFX font and determine the number of pixels
// needed to display the characters.
//----------------------------------------------------------------------
void GetGFXFontMetrics(const GFXfont* f, int* LineDepth, int* CharDepth, int* Descender) {
  // The line depth is the vertical space moved by \n and it's directly in the font
  if (LineDepth)
    *LineDepth = f->yAdvance;

  // To get the character depth we need to scan all the characters
  // Go through the character glyphs array to find the spacing and
  // descender size;
  int ascent = 0, descent = 0;
  int num_chars = f->last - f->first + 1;

  for (int i = 0; i < num_chars; i++) {
    // This is the pixels above the base
    int asc = -f->glyph[i].yOffset;
    if (asc > ascent)
      ascent = asc;
  
    // This is the pixels below the base i.e. the descender size
    int desc = f->glyph[i].height + f->glyph[i].yOffset;
    if (desc > descent)
      descent = desc;
  }

  // And return the values
  if (CharDepth)
    *CharDepth = ascent + descent;
  if (Descender)
    *Descender = descent;
}

//----------------------------------------------------------------------
// TestFont
// --------
// Fill the screen with text in the current font
//----------------------------------------------------------------------
void TestFont(const GFXfont* Font) {
    tft.fillScreen(COL_BACK);

  // Get the font metrics
  int line_depth, char_depth, descender;
  GetGFXFontMetrics(Font, &line_depth, &char_depth, &descender);
  Serial.printf("line_depth = %d, char_depth = %d, descender = %d\n", line_depth, char_depth, descender);

  // Render the text with a coloured background
  int max_lines = SCREEN_DEPTH/line_depth;
  uint16_t cur_colour = ILI9341_BLUE;

  for (int i = 0; i < max_lines; i++) {
    // Set the next colour
    if (cur_colour == ILI9341_BLUE)
      cur_colour = ILI9341_RED;
    else if (cur_colour == ILI9341_RED)
       cur_colour = ILI9341_GREEN;
    else
       cur_colour = ILI9341_BLUE;

    // Erase the current line
    int base = (i+1)*line_depth;
    tft.fillRect(0, base-line_depth+descender, SCREEN_WIDTH, line_depth, cur_colour);

    // Write the current line
    tft.setFont(Font);
    tft.setCursor(0, base);
    tft.printf(TEST_TEXT);
  }
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
  TestFont(&FreeSans9pt7b);
  Serial.println("Press enter to continue");
  String s = Serial.readStringUntil(10);

  // 12pt Sans
  Serial.println("Testing FreeSans12pt7b");
  TestFont(&FreeSans12pt7b);
  Serial.println("Press enter to continue");
  Serial.readStringUntil(10);

  // 18pt Sans
  Serial.println("Testing FreeSans18pt7b");
  TestFont(&FreeSans18pt7b);
  Serial.println("Press enter to continue");
  Serial.readStringUntil(10);

  // 24pt Sans
  Serial.println("Testing FreeSans24pt7b");
  TestFont(&FreeSans24pt7b);
  Serial.println("Press enter to continue");
  Serial.readStringUntil(10);

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