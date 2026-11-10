//----------------------------------------------------------------------
// TFT_eSPI
// ========
// This is a basic example of using TFT_eSPI as the display driver for
// the LVGL library.
//----------------------------------------------------------------------
#include <lvgl.h>
#include "ILI9341.h"

// Screen size
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

// Buffer used by LVGL
uint8_t* drawBuf = NULL; // DRAW_BUF_SIZE

// Text area
static lv_obj_t* txt;

// Prototype for the display flush function
void lcd_flush_cb(lv_display_t * display, const lv_area_t * area, uint8_t * px_map);

//----------------------------------------------------------------------
// log_print
// ---------
// If logging is enabled, it will inform the user about what is happening in the library
//----------------------------------------------------------------------
void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

//----------------------------------------------------------------------
// CreateMainGUI
// -------------
// Create the controls on the display
//----------------------------------------------------------------------
void CreateMainGUI(void) {
  // Create the text area
  txt = lv_textarea_create(lv_screen_active());
  // Resize the text area to fill the screen with a 5 pixel margin
  lv_obj_set_pos(txt, 5, 5);
  lv_obj_set_size(txt, SCREEN_WIDTH-10, SCREEN_HEIGHT-10);
  // Set the textarea event handler (currently unused)
  // lv_obj_add_event_cb(txt, textarea_event_handler, LV_EVENT_READY, txt);
  // Set the focus to the text area
  lv_obj_add_state(txt, LV_STATE_FOCUSED);
}

//----------------------------------------------------------------------
// ProcessInput
// ------------
// Add the string read from the serial monitor input to the edit control
//----------------------------------------------------------------------
void ProcessInput(const char* Input) {
  lv_textarea_add_text(txt, Input);
}

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup() {
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(LVGL_Arduino);

  // Initialise the LCD driver
  if (!LCDInit()) {
    Serial.println("LCD initialisation failed");
    vTaskSuspend(NULL);
  }
  Serial.println("LCD initialised");

  // Allocate the drawing buffer
  uint8_t* drawBuf = (uint8_t*) heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_DMA);
  if (!drawBuf) {
    Serial.println("Cannot allocate draw buffer");
    vTaskSuspend(NULL);
  }

  // Start LVGL
  lv_init();
  // Register print function for debugging
  lv_log_register_print_cb(log_print);

  // Create a display
  lv_display_t* display = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
  // lv_display_set_rotation(display, LV_DISPLAY_ROTATION_0);
  // And set the buffer that the display will use
  lv_display_set_buffers(display, drawBuf, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
  // And finally set the function that will render the data to the display
  lv_display_set_flush_cb(display, LCDFlush);
  Serial.println("Display created");

  // Draw the GUI
  CreateMainGUI();
  Serial.println("GUI created");
}

//----------------------------------------------------------------------
// loop
// ----
//----------------------------------------------------------------------
void loop() {
  // If there is input from the user read it and append it to the edit control
  if (Serial.available()) {
    // Read a string from the serial monitor
    String s = Serial.readString();
    // Send the string to the display
    ProcessInput(s.c_str());
  }

  lv_task_handler();  // let the GUI do its work
  lv_tick_inc(5);     // tell LVGL how much time has passed
  delay(5);           // let this time pass
}