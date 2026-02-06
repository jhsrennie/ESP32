//----------------------------------------------------------------------
// CYDConsole
// ==========
//----------------------------------------------------------------------
#ifndef __CYDCONSOLE_INC__
#define __CYDCONSOLE_INC__

// Screen size
#define SCREEN_WIDTH 320
#define SCREEN_DEPTH 240

// Colours
#define COL_BACK TFT_BLACK
#define COL_FORE TFT_WHITE

// Console font
#define CONSOLE_FONT         2 // Built in 16 pixel font
#define CONSOLE_FONT_DEPTH  16

// With the 16 pixel font we get 15 lines on the screen
#define CONSOLE_NUM_LINES   15
#define CONSOLE_BOTTOM_LINE (CONSOLE_NUM_LINES-1)

// Screen buffer string length
#define CONSOLE_BUF_LEN    256

class CYDConsole {
  public:
    CYDConsole();
    void init();
    void printf(const char* s, ...);

  private:
    int cur_line;
    int console_buf_pos;
    char console_buf[CONSOLE_NUM_LINES][CONSOLE_BUF_LEN];
};

#endif // __CYDCONSOLE_INC__