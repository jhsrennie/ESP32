//----------------------------------------------------------------------
// CYDConsole
// ==========
//----------------------------------------------------------------------
#include <TFT_eSPI.h>
#include <CYDConsole.h>

// We use a single global graphics object
TFT_eSPI tft = TFT_eSPI();

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

  // Start the tft display and set it to black
  tft.init();
  // My display needs the colours inverted for some reason
  tft.invertDisplay(true);
  // This is the display in landscape with the origin at top left
  tft.setRotation(3);
  
  // Clear the screen before writing to it
  tft.fillScreen(COL_BACK);
  tft.setTextColor(COL_FORE);
  tft.setTextFont(CONSOLE_FONT);
  tft.setTextSize(1);
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
  if (console_buf_pos >= CONSOLE_NUM_LINES)
    console_buf_pos = 0;

  // If necessary scroll the screen up
  if (cur_line == CONSOLE_NUM_LINES) {
    for (int i = 0; i < CONSOLE_NUM_LINES-1; i++) {
      int line = (i + console_buf_pos) % CONSOLE_NUM_LINES;
      tft.fillRect(0, i*CONSOLE_FONT_DEPTH, SCREEN_WIDTH, CONSOLE_FONT_DEPTH, COL_BACK);
      tft.drawString(console_buf[line], 0, i*CONSOLE_FONT_DEPTH);
    }
    cur_line--;
  }
  // Display the message
  int cur_pos = cur_line*CONSOLE_FONT_DEPTH;
  tft.fillRect(0, cur_pos, SCREEN_WIDTH, CONSOLE_FONT_DEPTH*2, COL_BACK);
  tft.drawString(s, 0, cur_pos);
  cur_line++;
}