//----------------------------------------------------------------------
// CYDConsole
// ==========
// Class to implement a simple scrolling console on a CYD
//----------------------------------------------------------------------
#ifndef __CYDCONSOLE_INC__
#define __CYDCONSOLE_INC__

// With the 9 point font we get 10 lines on the screen
#define CONSOLE_NUM_LINES   10

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