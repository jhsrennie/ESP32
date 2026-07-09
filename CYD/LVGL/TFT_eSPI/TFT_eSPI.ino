//----------------------------------------------------------------------
// TFT_eSPI
// ========
// Basic example showing how to use the LVGL library
// This file includes all the initialisation code required to use LVGL.
// The code for the user interface is in LVGL_UI.cpp.
//----------------------------------------------------------------------
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// Touchscreen pins
#define XPT2046_IRQ  36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK  25
#define XPT2046_CS   33

// The touch positions have a border
#define TOUCH_X_MIN 250
#define TOUCH_X_MAX 3800
#define TOUCH_Y_MIN 250
#define TOUCH_Y_MAX 3900
// On my CYD 2 sets the touch origin at the bottom left (as LVGL
// requires) when in landscape with the USB on the left.
#define XPT2046_ROTATION 2

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

// Set to your screen resolution and rotation
// On my CYD you have to set these as if the screen was in portrait mode
// even when you're using landscape.
#define TFT_HOR_RES   240
#define TFT_VER_RES   320
// On my CYD 270 degree rotation sets landscape with the USB connector
// on the left.
#define TFT_ROTATION  LV_DISPLAY_ROTATION_270

// LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes.
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

// UI code from LVGL_UI.cpp
void CreateExampleScreen();

//----------------------------------------------------------------------
// my_print
// --------
// Function used by LVGL to send log messages.
// Call lv_log_register_print_cb() to set this as the logging callback.
//----------------------------------------------------------------------
void my_print(lv_log_level_t level, const char * buf)
{
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

//----------------------------------------------------------------------
// my_touchpad_read
// ----------------
// Function used by LVGL to read data from the touch control. Call
// lv_indev_set_read_cb() to set this as the touch callback.
// The XPT2046 returns the touch position as a 12 bit number i.e. 0 to
// 4095. We need to map this to screen pixel position i.e. 0 to
// TFT_HOR_RES-1 and 0 to TFT_VER_RES-1. The rotation has to be set to
// put the origin at the top left, which is typically done by
// experiment.
//----------------------------------------------------------------------
void my_touchpad_read(lv_indev_t * indev, lv_indev_data_t * data) {
  int x, y, pressure;

  // Check if the touchscreen has been touched
  if(!touchscreen.tirqTouched() || !touchscreen.touched()) {
    data->state = LV_INDEV_STATE_RELEASED;
    return;
  }

  // Get Touchscreen points
  TS_Point p = touchscreen.getPoint();

  // Uncomment this to display the touch info for calibration
  // static int pxmin = 2048, pxmax = 2048, pymin = 2048, pymax = 2048;
  // if (p.x < pxmin) pxmin = p.x;
  // if (p.x > pxmax) pxmax = p.x;
  // if (p.y < pymin) pymin = p.y;
  // if (p.y > pymax) pymax = p.y;
  // Serial.printf("p = (%d, %d), min = (%d, %d), max = (%d, %d)\n", p.x, p.y, pxmin, pymin, pxmax, pymax);

  // Convert the touch coordinates to pixel coordinates
  x = (TFT_HOR_RES*(p.x - TOUCH_X_MIN))/(TOUCH_X_MAX - TOUCH_X_MIN);
  y = (TFT_VER_RES*(p.y - TOUCH_Y_MIN))/(TOUCH_Y_MAX - TOUCH_Y_MIN);

  // LVGL gets really unhappy if you pass a touch position off the screen
  if (x < 0 || x >= TFT_HOR_RES) {
    Serial.printf("x is out of range: %d\n", x);
    x = 0;
  }
  if (y < 0 || y >= TFT_VER_RES) {
    Serial.printf("y is out of range: %d\n", y);
    y = 0;
  }

  pressure = p.z;

  data->state = LV_INDEV_STATE_PRESSED;
  data->point.x = x;
  data->point.y = y;

  // Print Touchscreen info about X, Y and Pressure (Z) on the Serial Monitor
  // Serial.printf("x = %d, y = %d, pressure = %d\n", x, y, pressure);
}

//----------------------------------------------------------------------
// my_tick
// -------
// Function used by LVGL to get the elapsed time in milliseconds.
// Call lv_tick_set_cb() to set this as the time callback.
//----------------------------------------------------------------------
static uint32_t my_tick(void) {
  return millis();
}

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup()
{
  String LVGL_Arduino = "Hello Arduino! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

  Serial.begin(115200);
  Serial.println(LVGL_Arduino);

  lv_init();

  // Set the elapsed time callback
  lv_tick_set_cb(my_tick);

  // Set the logging callback
  lv_log_register_print_cb(my_print);

  // Create the display
  lv_display_t * disp;
  disp = lv_tft_espi_create(TFT_HOR_RES, TFT_VER_RES, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, TFT_ROTATION);

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(XPT2046_ROTATION);

  // Initialize the (dummy) input device driver
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); // Touchpad should have POINTER type
  // Set the touch callback
  lv_indev_set_read_cb(indev, my_touchpad_read);

  // Create the UI
  CreateExampleScreen();

  // All done
  Serial.println("Setup done");
}

//----------------------------------------------------------------------
// loop
// ----
//----------------------------------------------------------------------
void loop() {
  // Call this to allow LVGL to process any events
  lv_timer_handler();
  // A 5ms latency between LVG events is about right
  delay(5);
}