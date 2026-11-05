//----------------------------------------------------------------------
// LVGLText
// ========
// This is a basic example of using TFT_eSPI or LovyanGFX as the display
// driver for the LVGL library.
//----------------------------------------------------------------------
#include <lvgl.h>

// Uncomment to use either TFT_eSPI or Lovyan as the display driver
#define USE_TFT_ESPI 1
// #define USE_LOVYANGFX 1

#if defined USE_TFT_ESPI
#include <TFT_eSPI.h>
#elif defined USE_LOVYANGFX
#include <LovyanGFX.hpp>
#else
#error Unknown library
#endif

// Screen size
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// Buffer used by LVGL
// The usual recommendation is to use 10% of the screen area
#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint8_t drawBuf[DRAW_BUF_SIZE];

// Text area
static lv_obj_t* txt;

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
  lv_obj_set_size(txt, 310, 230);
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

  // Start LVGL
  lv_init();
  // Register print function for debugging
  lv_log_register_print_cb(log_print);

  // Create a display object
  lv_display_t * disp;
  // Initialize the TFT display using the TFT_eSPI library
#ifdef USE_TFT_ESPI
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, drawBuf, sizeof(drawBuf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
  Serial.println("TFT_eSPI initialised");
#elif defined USE_LOVYANGFX
  disp = lv_lovyan_gfx_create(SCREEN_WIDTH, SCREEN_HEIGHT, drawBuf, sizeof(drawBuf), true);
  Serial.println("Loyyan initialised");
#endif
  
  // Function to draw the GUI
  CreateMainGUI();
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