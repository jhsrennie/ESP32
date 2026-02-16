//----------------------------------------------------------------------
// CYDConsole
// ==========
// Class to implement a simple scrolling console on a CYD
//----------------------------------------------------------------------
#ifndef __CYDCONSOLE_INC__
#define __CYDCONSOLE_INC__

// Screen buffer string length
#define CONSOLE_BUF_LEN    256

class CYDConsole {
  public:
    CYDConsole();
    void init();
    void printf(const char* s, ...);

    // getters
    int maxLines() const { return max_lines; }

  private:
    int font_depth, font_descender;
    int max_lines;
    int cur_line;
    int console_buf_pos;
    char console_buf[25][CONSOLE_BUF_LEN];
};

#endif // __CYDCONSOLE_INC__