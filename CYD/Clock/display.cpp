//----------------------------------------------------------------------
// display.cpp
// ===========
// Display functions.
// This has been written so it can use either the TFT_eSPI library or
// the ArduinoGFX library. They are similar enough that only a few
// conditional blocks are needed.
//----------------------------------------------------------------------
// Use this define to selet the TFT_eSPI library. If this is not defined
// the ArduinoGFX library is used instead.
#define UseTFT_eSPI

#include <driver/ledc.h>
#include <XPT2046_Touchscreen.h>
#include "clock.h"

// Choose the graphics library
#ifdef UseTFT_eSPI
#include <TFT_eSPI.h>
#else
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#endif

// Include the Arduino GFX fonts if necessary
#ifndef UseTFT_eSPI
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#endif

// Local font for the 7 segment display
#include "DSEG7_Classic_Bold_48.h"
// Font sizes in pixels
#include "FontMetricsGFX.h"

// TFT_eSPI uses the setFreeFont() function instead of setFont() for the free fonts
#ifdef UseTFT_eSPI
#define setFont setFreeFont
#endif

// LEDC settings
#define LEDC_PIN                     21
#define LEDC_FREQ                  1000
#define XTAL_CLK_FREQ          80000000
#define SPEED_MODE LEDC_HIGH_SPEED_MODE
#define CLK_CFG    LEDC_USE_APB_CLK

// Colours
#ifdef UseTFT_eSPI
#define COL_BACK TFT_BLACK
#define COL_FORE TFT_WHITE
#else
#define COL_BACK ILI9341_BLACK
#define COL_FORE ILI9341_WHITE
#endif

// Screen size
#define SCREEN_WIDTH 320
#define SCREEN_DEPTH 240

// Size of the 48pt 7 segment font
#define DEPTH_48PT   53
#define VOFFSET_48PT  1

// Position of time
#define TIME_FONT &DSEG7_Classic_Bold_48
#define TIME_HEIGHT  53 // Size in pixels of the font
#define TIME_VOFFSET  1 // Offset needed to leave 1 pixel below the lowest descender
#define TIME_LEFT    20
#define TIME_BASE    75
#define TIME_SPACE   90 // Space between "hh:", "mm:" and "ss:"

#define DATE_FONT &FreeSans12pt7b
#define DATE_HEIGHT  DEPTH_12PT
#define DATE_VOFFSET VOFFSET_12PT
#define DATE_LEFT    60
#define DATE_BASE   105

#define TEMP_FONT  &FreeSans24pt7b
#define TEMP_FONT2 &FreeSans9pt7b // used for degree symbol
#define TEMP_HEIGHT  DEPTH_24PT
#define TEMP_VOFFSET VOFFSET_24PT
#define TEMP_LEFT     10
#define TEMP_BASE    180
#define TEMP_WIDTH   160

#define HUMID_FONT &FreeSans12pt7b
#define HUMID_HEIGHT  DEPTH_12PT
#define HUMID_VOFFSET VOFFSET_12PT
#define HUMID_LEFT   170
#define HUMID_BASE   190
#define HUMID_SPACE   70 // Distance from HUMID_LEFT to numbers

#define STATUS_FONT &FreeSans9pt7b
#define STATUS_HEIGHT  DEPTH_9PT
#define STATUS_VOFFSET VOFFSET_9PT
#define STATUS_LEFT    0
#define STATUS_BASE    (SCREEN_DEPTH-STATUS_VOFFSET)

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

// Create the global touchscreen object
#define XPT2046_IRQ  36  // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK  25  // T_CLK
#define XPT2046_CS   33  // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

//----------------------------------------------------------------------
// InitDisplay
// -----------
//----------------------------------------------------------------------
void InitDisplay() {
  // Initialise the display library
#ifdef UseTFT_eSPI
  tft.init();
  tft.setRotation(3);
#else
  tft.begin();
  tft.setRotation(0);
#endif
  tft.fillScreen(COL_BACK);

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
}

//----------------------------------------------------------------------
// DisplayStatus
//--------------
// This writes a status message at the bottom of the screen
//----------------------------------------------------------------------
void DisplayStatus(char *Format, ...) {
  // Clear the status area
  tft.fillRect(0, STATUS_BASE-STATUS_HEIGHT+STATUS_VOFFSET, SCREEN_WIDTH, STATUS_HEIGHT, COL_BACK);

  // Format the message
  char s[256];
  va_list ap;
  va_start(ap, Format);
  vsnprintf(s, 256, Format, ap);
  va_end(ap);

  // Display the message
  tft.setFont(STATUS_FONT);
  tft.setCursor(STATUS_LEFT, STATUS_BASE);
  tft.print(s);
}

//----------------------------------------------------------------------
// DisplayTime
//------------
//----------------------------------------------------------------------
void DisplayTime() {
  // This keeps the last time written.
  // We need this so we can erase the last time written by overwriting
  // it in the background colour.
  static struct tm tm_last = {0};

  // Get the time. We have not set a time zone so the time is GMT.
  struct timeval tv_now;
  gettimeofday(&tv_now, NULL);
  time_t tt = (time_t) tv_now.tv_sec;
  struct tm* tm_now = localtime(&tt);

  // Format and print the time
  tft.setFont(TIME_FONT);

  // write the hour
  if (tm_last.tm_hour != tm_now->tm_hour) {
    tft.fillRect(TIME_LEFT, TIME_BASE-TIME_HEIGHT+TIME_VOFFSET, TIME_SPACE, TIME_HEIGHT, COL_BACK);
    tft.setTextColor(COL_FORE);
    tft.setCursor(TIME_LEFT, TIME_BASE);
    tft.printf("%02d:", tm_now->tm_hour);
  }

  // write the minute
  if (tm_last.tm_min != tm_now->tm_min) {
    tft.fillRect(TIME_LEFT+TIME_SPACE, TIME_BASE-TIME_HEIGHT+TIME_VOFFSET, TIME_SPACE, TIME_HEIGHT, COL_BACK);
    tft.setTextColor(COL_FORE);
    tft.setCursor(TIME_LEFT+TIME_SPACE, TIME_BASE);
    tft.printf("%02d:", tm_now->tm_min);
  }

  // write the second
  if (tm_last.tm_sec != tm_now->tm_sec) {
    tft.fillRect(TIME_LEFT+2*TIME_SPACE, TIME_BASE-TIME_HEIGHT+TIME_VOFFSET, TIME_SPACE, TIME_HEIGHT+2, COL_BACK);
    tft.setTextColor(COL_FORE);
    tft.setCursor(TIME_LEFT+2*TIME_SPACE, TIME_BASE);
    tft.printf("%02d", tm_now->tm_sec);
  }

  // Print the date on the next line
  if (tm_last.tm_mday != tm_now->tm_mday || tm_last.tm_mon != tm_now->tm_mon || tm_last.tm_year != tm_now->tm_year) {
    tft.fillRect(0, DATE_BASE-DATE_HEIGHT+DATE_VOFFSET, SCREEN_WIDTH, DATE_HEIGHT, COL_BACK);
    #define LEN_BUF 256
    char buf[LEN_BUF];
    strftime(buf, LEN_BUF, "%a, %d %b %Y", tm_now);

    tft.setFont(DATE_FONT);
    tft.setCursor(DATE_LEFT, DATE_BASE);
    tft.print(buf);
  }

  // Save the time so it can be erased next loop
  tm_last = *tm_now;
}

//----------------------------------------------------------------------
// DisplayTemp
//------------
// This displays the temperature, humidity and pressure
//----------------------------------------------------------------------
void DisplayTemp() {
  // Erase the previous temperature
  tft.fillRect(TEMP_LEFT, TEMP_BASE-TEMP_HEIGHT+TEMP_VOFFSET, TEMP_WIDTH, TEMP_HEIGHT, COL_BACK);

  // Display the temp only if the AHT20 was initialised
  if (aht_init) {
    tft.setCursor(TEMP_LEFT, TEMP_BASE);
    tft.setFont(TEMP_FONT);
    tft.printf("%.1f", Temperature);
    // We need to move the cursor up for the degrees symbol
    tft.setCursor(tft.getCursorX(), TEMP_BASE-25);
    tft.setFont(TEMP_FONT2);
    tft.print("o");
    // Then move it down again
    tft.setCursor(tft.getCursorX(), TEMP_BASE);
    tft.setFont(TEMP_FONT);
    tft.print("C");
  }

  // Humidity and pressure
  tft.fillRect(HUMID_LEFT, HUMID_BASE-2*HUMID_HEIGHT+HUMID_VOFFSET, SCREEN_WIDTH-HUMID_LEFT, 2*HUMID_HEIGHT, COL_BACK);
  tft.setFont(HUMID_FONT);

  // Display the humidity only if the AHT20 was initialised
  if (aht_init) {
    tft.setCursor(HUMID_LEFT, HUMID_BASE-HUMID_HEIGHT);
    tft.print("RH");
    tft.setCursor(HUMID_LEFT+HUMID_SPACE, HUMID_BASE-HUMID_HEIGHT);
    tft.printf("%d%%", (int) Humidity);
  }

  // Display the pressure only if the BMP280 was initialised
  if (bmp_init) {
    tft.setCursor(HUMID_LEFT, HUMID_BASE);
    tft.print("Press");
    tft.setCursor(HUMID_LEFT+HUMID_SPACE, HUMID_BASE);
    // Is the preesure in Pascals or 10^5 atm?
    // tft.printf("%d", (int) (Pressure/101.325));
    tft.printf("%d", (int) (Pressure));
  }
}

//----------------------------------------------------------------------
// SetBrightness
//--------------
// The brightness is a value from 0 to 100
// These settings are for the ESP32. The settings are different for
// other ESP32 variants.
// We set the resolution bit to 16 = 65536 so the duty setting is
// Brightness*65536/100
//----------------------------------------------------------------------
void SetBrightness(int Brightness) {
  static uint32_t resolution;

  // We only need to configure the channel once
  static bool configured = false;
  if (!configured) {
    resolution = ledc_find_suitable_duty_resolution(XTAL_CLK_FREQ, LEDC_FREQ);
    ledc_timer_config_t ledc_timer = {};
    ledc_timer.speed_mode      = LEDC_HIGH_SPEED_MODE;
    ledc_timer.duty_resolution = ledc_timer_bit_t(resolution);
    ledc_timer.timer_num       = LEDC_TIMER_0;
    ledc_timer.freq_hz         = LEDC_FREQ;
    ledc_timer.clk_cfg         = LEDC_USE_APB_CLK;
    ledc_timer_config(&ledc_timer);
    configured = true;
  }

  // Configure the channel
  ledc_channel_config_t ledc_channel = {0};
  ledc_channel.channel    = LEDC_CHANNEL_0;
  ledc_channel.duty       = (Brightness << resolution)/100;
  ledc_channel.gpio_num   = LEDC_PIN;
  ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
  ledc_channel.timer_sel  = LEDC_TIMER_0;
  ledc_channel_config(&ledc_channel);
}

//----------------------------------------------------------------------
// IsTouched
// ---------
// Return true if the display has been touched
//----------------------------------------------------------------------
bool IsTouched() {
  return touchscreen.tirqTouched() && touchscreen.touched();
}