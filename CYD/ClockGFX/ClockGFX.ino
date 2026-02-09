//----------------------------------------------------------------------
// CYD Clock program
// =================
//----------------------------------------------------------------------
#include <WiFi.h>
#include <esp_netif_sntp.h>
#include <driver/ledc.h>
#include <time.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

// Arduino GFX fonts
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
// Local font for the 7 segment display
#include "DSEG7_Classic_Bold_48.h"
// Font sizes in pixels
#include "FontMetricsGFX.h"

// Sensors
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>

#define GPIO_SDA 22
#define GPIO_SCL 27
Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;
bool aht_init = true, bmp_init = true;
float Temperature = 0, Humidity = 0, Pressure = 0;

// WiFi credentials
#define WIFI_SSID "MYSSID"
#define WIFI_PWD  "MYPASS"

// NTP server details
#define TIMEZONE  "GMT0BST,M3.5.0/1,M10.5.0"
#define NTPSERVER "pool.ntp.org"
#define NTP_RETRIES 3
#define NTP_FAILS   3

// LEDC settings
#define LEDC_PIN                     21
#define LEDC_FREQ                  1000
#define XTAL_CLK_FREQ          80000000
#define SPEED_MODE LEDC_HIGH_SPEED_MODE
#define CLK_CFG    LEDC_USE_APB_CLK

// Colours
#define COL_BACK ILI9341_BLACK
#define COL_FORE ILI9341_WHITE

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
#define TFT_CS   15 
#define TFT_DC    2
#define TFT_RST  -1

SPIClass tftSPI = SPIClass(HSPI);
Adafruit_ILI9341 tft = Adafruit_ILI9341(&tftSPI, TFT_DC, TFT_CS, TFT_RST);

// Create the global touchscreen object
#define XPT2046_IRQ  36  // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK  25  // T_CLK
#define XPT2046_CS   33  // T_CS

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

//----------------------------------------------------------------------
// UpdateTime
//-----------
//----------------------------------------------------------------------
void UpdateTime(void* Unused) {
  #define MAX_RETRIES 3
  int time_retries;

  // Initialise the SNTP system
  esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(NTPSERVER);

  // Loop continuously updating the time once an hour
  for (;;) {
    // Enable the wi-fi
    DisplayStatus("Connecting to WiFi ...");
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(WIFI_PS_NONE);
    WiFi.begin(WIFI_SSID, WIFI_PWD);
    int loopcnt = 0;
    Serial.println("Connecting to WiFi ...");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.printf("Connecting: time %d, WiFi status = %d, signal = %d\n", loopcnt++, WiFi.status(), WiFi.RSSI());
      delay(1000);
    }
    Serial.println(WiFi.localIP());

    // Get the time
    Serial.println("Getting the time ...");
    DisplayStatus("Getting the time ...");

    for (time_retries = 0; time_retries < MAX_RETRIES; time_retries++) {
      Serial.printf("Attempt %d ... ", time_retries);

      // Call init to start the sync
      esp_netif_sntp_init(&config);
      configTzTime(TIMEZONE, NTPSERVER);

      // And wait for the sync to complete
      esp_err_t e = esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000));
      // Deinitialise after the sync has completed
      esp_netif_sntp_deinit();

      // Check if the sync succeeded
      if (e == ESP_OK) {
        Serial.println("succeeded");
        struct timeval tv_now;
        gettimeofday(&tv_now, NULL);
        time_t tt = (time_t) tv_now.tv_sec;
        struct tm* tm_now = localtime(&tt);
        DisplayStatus("Synced with %s at %02d:%02d:%02d", NTPSERVER, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
        // And exit the loop since we succeeded
        break;
      }
      else {
        Serial.printf("failed: %s\n", esp_err_to_name(e));
        DisplayStatus("Time sync failed");
      }
    }

    // Disable the wi-fi to save power
    WiFi.disconnect();

    // And wait an hour
    delay(3600*1000);
  }
}

//----------------------------------------------------------------------
// InitSensors
// -----------
// Initialise the AHT20 and BMP280
//----------------------------------------------------------------------
void InitSensors() {
  // Set the I2C pins that the CYD uses
  Wire.setPins(GPIO_SDA, GPIO_SCL);
  Wire.begin();

  // Initialise the AHT20
  if (aht.begin()) {
    Serial.println("AHT20 initialised");
    aht_init = true;
  }
  else {
    Serial.println("AHT20 not found");
    aht_init = false;
  }

  if (bmp.begin()) {
    // Default settings from datasheet
    bmp.setSampling(
      Adafruit_BMP280::MODE_NORMAL,     // Operating Mode
      Adafruit_BMP280::SAMPLING_X2,     // Temp. oversampling
      Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
      Adafruit_BMP280::FILTER_X16,      // Filtering
      Adafruit_BMP280::STANDBY_MS_500); // Standby time
    Serial.println("BMP280 initialised");
    bmp_init = true;
  }
  else {
    Serial.println("BMP280 not found");
    bmp_init = false;
  }
}

//----------------------------------------------------------------------
// GetTemp
//--------
// read the temperature, etc from the AHT11
//----------------------------------------------------------------------
void GetTemp() {
  // Query the AHT20
  if (aht_init) {
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    Serial.printf("Temp: %f, humidity: %f\n", temp.temperature, humidity.relative_humidity);

    Temperature = temp.temperature;        // Temp is in Celsius
    Humidity = humidity.relative_humidity; // RH is in %
  }

  // Query the BMP280
  if (bmp_init) {
    float bmp_temp = bmp.readTemperature();  // Temp is in Celsius
    float bmp_pressure = bmp.readPressure(); // Pressure is in Pa
    Serial.printf("Temp: %f, pressure: %f\n", bmp_temp, bmp_pressure);

    Pressure = bmp_pressure;
  }
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
    tft.printf("%d", (int) (Pressure/101.325));
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
// setup
//------
//----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting Clock");

  // Initialise the display
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(COL_BACK);
  DisplayStatus("Starting clock ...");

  // Start the task to get the time
  TaskHandle_t h;
  xTaskCreate(UpdateTime, "UpdateTime", 4000, NULL, 2, &h);

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);

  // Initialise the sensors
  Serial.println("Initialising sensors");
  InitSensors();
}

//----------------------------------------------------------------------
// loop
//-----
//----------------------------------------------------------------------
void loop() {
  static int brightness = 100;

  // The time updates every loop
  static int timecount = 0;
  if (timecount == 0)
    DisplayTime();
  timecount = (timecount + 1) % 10;

  // If the touchscreen is touched ramp up the brightness
  if(touchscreen.tirqTouched() && touchscreen.touched())
    brightness = 100;

  // Gradually dim the screen
  if (brightness >= 5)
    SetBrightness(brightness);
  if (brightness > 0)
    brightness -= 1;

  // The temperature updates every 60 seconds
  static int tempcount = 0;
  if (tempcount == 0) {
    GetTemp();
    DisplayTemp();
  }
  tempcount = (tempcount + 1) % 600;

  delay(100);
}