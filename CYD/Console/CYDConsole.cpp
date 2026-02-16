//----------------------------------------------------------------------
// CYDConsole
// ==========
// Class to implement a simple scrolling console on a CYD
//----------------------------------------------------------------------
// Use this define to select the TFT_eSPI library. If this is not defined
// the ArduinoGFX library is used instead.
#define UseTFT_eSPI

// Choose the graphics library
#ifdef UseTFT_eSPI
#include <TFT_eSPI.h>
// TFT_eSPI uses the setFreeFont() function instead of setFont() for the free fonts
#define setFont setFreeFont
#else
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#endif

#include <CYDConsole.h>

// Set the font used for the console
#ifndef UseTFT_eSPI
#include <Fonts/FreeSans9pt7b.h>
#endif
#define CONSOLE_FONT &FreeSans9pt7b

// Screen size
#define SCREEN_WIDTH 320
#define SCREEN_DEPTH 240

// Colours
#define COL_BACK ILI9341_BLACK
#define COL_FORE ILI9341_WHITE

// Backlight pin
#define PIN_LED 21

// Create the global display object
#ifdef UseTFT_eSPI
TFT_eSPI tft = TFT_eSPI();
#else
#define TFT_CS   15 
#define TFT_DC    2
#define TFT_RST  -1
SPIClass tftSPI = SPIClass(HSPI);
Adafruit_ILI9341 tft = Adafruit_ILI9341(&tftSPI, TFT_DC, TFT_CS, TFT_RST);
#endif

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
  int char_depth = 0, descender = 0;
  int num_chars = f->last - f->first + 1;

  for (int i = 0; i < num_chars; i++) {
    // This is the pixels above the base
    int depth = -f->glyph[i].yOffset;
    if (depth > char_depth)
      char_depth = depth;
  
    // This is the pixels below the base i.e. the descender size
    int desc = f->glyph[i].height + f->glyph[i].yOffset;
    if (desc > descender)
      descender = desc;
  }

  // Finally add the descender size to the max offset to get the total character depth
  char_depth += descender;

  // And return the values
  if (CharDepth)
    *CharDepth = char_depth;
  if (Descender)
    *Descender = descender;
}

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

  // Get the font size
  GetGFXFontMetrics(CONSOLE_FONT, NULL, &font_depth, &font_descender);
  max_lines = SCREEN_DEPTH/font_depth;

  // Turn the backlight on
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  // Initialise the display
#ifdef UseTFT_eSPI
  tft.init();
  tft.invertDisplay(true);
  tft.setRotation(3);
#else
  tft.begin();
  tft.setRotation(0);
#endif
  
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

  // And copy it into the screen buffer
  strlcpy(console_buf[console_buf_pos++], s, CONSOLE_BUF_LEN);
  if (console_buf_pos >= max_lines)
    console_buf_pos = 0;

  // If necessary scroll the screen up
  if (cur_line == max_lines) {
    for (int i = 0; i < max_lines-1; i++) {
      int line = (i + console_buf_pos) % max_lines;
      tft.fillRect(0, i*font_depth, SCREEN_WIDTH, font_depth, COL_BACK);
      tft.setCursor(0, (i+1)*font_depth - font_descender);
      tft.print(console_buf[line]);
    }
    cur_line--;
  }
  // Display the message
  tft.fillRect(0, cur_line*font_depth, SCREEN_WIDTH, font_depth, COL_BACK);
  tft.setCursor(0, (cur_line+1)*font_depth - font_descender);
  tft.print(s);
  cur_line++;
}