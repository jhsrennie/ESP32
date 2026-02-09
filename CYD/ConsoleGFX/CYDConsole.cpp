//----------------------------------------------------------------------
// CYDConsole
// ==========
// Class to implement a simple scrolling console on a CYD
//----------------------------------------------------------------------
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <CYDConsole.h>
#include <Fonts/FreeSans9pt7b.h>
#include "FontMetricsGFX.h"

// Arduino GFX fonts
#define CONSOLE_FONT &FreeSans9pt7b
#define CONSOLE_FONT_DEPTH   DEPTH_9PT
#define CONSOLE_FONT_VOFFSET VOFFSET_9PT

// Screen size
#define SCREEN_WIDTH 320
#define SCREEN_DEPTH 240

// Colours
#define COL_BACK ILI9341_BLACK
#define COL_FORE ILI9341_WHITE

// Backlight pin
#define PIN_LED 21

// Create the global display object
#define TFT_CS   15 
#define TFT_DC    2
#define TFT_RST  -1

SPIClass tftSPI = SPIClass(HSPI);
Adafruit_ILI9341 tft = Adafruit_ILI9341(&tftSPI, TFT_DC, TFT_CS, TFT_RST);


CYDConsole::CYDConsole() {
  // Start at the top of the screen
  cur_line = 0;
  // Current buffer position
  console_buf_pos = 0;
}

void CYDConsole::init() {
  static bool initialised = false;
  if (initialised)
    return;

  // Turn the backlight on
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  // Initialise the display
  tft.begin();
  tft.setRotation(0);
  
  // Clear the screen before writing to it
  tft.fillScreen(COL_BACK);
  tft.setTextColor(COL_FORE);
  tft.setFont(CONSOLE_FONT);
  tft.setCursor(0, 0);

  // Start at the top of the screen
  cur_line = 0;
}

void CYDConsole::printf(const char* Format, ...) {
  // Format the message
  char s[256];
  va_list ap;
  va_start(ap, Format);
  vsnprintf(s, 256, Format, ap);
  va_end(ap);
  Serial.println(s);

  // And copy it into the screen buffer
  strlcpy(console_buf[console_buf_pos++], s, CONSOLE_BUF_LEN);
  if (console_buf_pos >= CONSOLE_NUM_LINES)
    console_buf_pos = 0;

  // If necessary scroll the screen up
  if (cur_line == CONSOLE_NUM_LINES) {
    for (int i = 0; i < CONSOLE_NUM_LINES-1; i++) {
      int line = (i + console_buf_pos) % CONSOLE_NUM_LINES;
      tft.fillRect(0, i*CONSOLE_FONT_DEPTH, SCREEN_WIDTH, CONSOLE_FONT_DEPTH, COL_BACK);
      tft.setCursor(0, (i+1)*CONSOLE_FONT_DEPTH - CONSOLE_FONT_VOFFSET);
      tft.print(console_buf[line]);
    }
    cur_line--;
  }
  // Display the message
  tft.fillRect(0, cur_line*CONSOLE_FONT_DEPTH, SCREEN_WIDTH, CONSOLE_FONT_DEPTH, COL_BACK);
  tft.setCursor(0, (cur_line+1)*CONSOLE_FONT_DEPTH - CONSOLE_FONT_VOFFSET);
  tft.print(s);
  cur_line++;
}