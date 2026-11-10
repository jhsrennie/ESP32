//----------------------------------------------------------------------
// ILI9341
// =======
// Driver for an ILI9341 display.
// This implements the flush_cb function usd by LVGL.
//----------------------------------------------------------------------

#ifndef _INC_ILI9341_FLUSHCB
#define _INC_ILI9341_FLUSHCB

// Maximum size of a data transfer
// The recommendation is 10% of the screen
#define DRAW_BUF_SIZE (2*SCREEN_WIDTH*SCREEN_HEIGHT/10)

bool LCDInit();
void LCDFlush(lv_display_t* display, const lv_area_t* area, uint8_t* px_map);

#endif // _INC_ILI9341_FLUSHCB