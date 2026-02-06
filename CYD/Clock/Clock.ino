//----------------------------------------------------------------------
// CYD Clock program
// =================
//----------------------------------------------------------------------
#include <WiFi.h>
#include <esp_netif_sntp.h>
#include <driver/ledc.h>
#include <time.h>
#include <TFT_eSPI.h>
#include <Free_Fonts.h>
#include <XPT2046_Touchscreen.h>

// WiFi credentials
#define WIFI_SSID "MYSSID"
#define WIFI_PWD  "MYPASS"

// NTP server details
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
#define COL_BACK TFT_BLACK
#define COL_FORE TFT_WHITE

// Screen size
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// Position of time
#define TIME_FONT     7 // 48 pixel 7 segment font
#define TIME_LEFT    52
#define TIME_TOP     30 // Use top for built in fonts
#define TIME_SPACE   75
#define TIME_HEIGHT  50


#define DATE_FONT FSS12 // The date is in 12 point sans serif
#define DATE_BASE   110 // Use base for free fonts
#define DATE_HEIGHT  30

#define TEMP_FONT  FSS24 // The temperature is in 24 point sans serif
#define TEMP_LEFT     10
#define TEMP_BASE    180 // Use base for free fonts
#define TEMP_WIDTH   100
#define TEMP_HEIGHT   50

#define HUMID_FONT FSS12 // The humidity and pressure are in 12 point sans serif
#define HUMID_LEFT   130
#define HUMID_BASE   190 // Use base for free fonts
#define HUMID_SPACE  110
#define HUMID_HEIGHT  30

#define STATUS_FONT   FSS9 // The status message is in 9 point sans serif
#define STATUS_LEFT      0
#define STATUS_BASE    235 // Use base for free fonts
#define STATUS_HEIGHT   20

// The display object
TFT_eSPI tft = TFT_eSPI();

// Touchscreen pins
#define XPT2046_IRQ  36  // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK  25  // T_CLK
#define XPT2046_CS   33  // T_CS

// Create the global touchscreen object
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

// Temperature etc
int Temperature, Humidity, Pressure;

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
// GetTemp
//--------
// read the temperature, etc from the AHT11
//----------------------------------------------------------------------
void GetTemp() {
  // For now make up random readings
  Temperature = 15 + micros() % 10;
  Humidity = 50 + micros() % 50;
  Pressure = 950 + micros() % 100;
}

//----------------------------------------------------------------------
// DisplayStatus
//--------------
// This writes a status message at the bottom of the screen
//----------------------------------------------------------------------
void DisplayStatus(char *Format, ...) {
  // Clear the status area
  tft.fillRect(0, STATUS_BASE-STATUS_HEIGHT+5, SCREEN_WIDTH, STATUS_HEIGHT, COL_BACK);
  tft.setFreeFont(STATUS_FONT);

  // Display the message
  char s[256];
  va_list ap;
  va_start(ap, Format);
  vsnprintf(s, 256, Format, ap);
  va_end(ap);

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
  tft.setTextFont(TIME_FONT);
  tft.setTextSize(1);

  // write the hour
  if (tm_last.tm_hour != tm_now->tm_hour) {
    tft.fillRect(TIME_LEFT, TIME_TOP, TIME_SPACE, TIME_HEIGHT, COL_BACK);
    tft.setTextColor(COL_FORE);
    tft.setCursor(TIME_LEFT, TIME_TOP);
    tft.printf("%02d:", tm_now->tm_hour);
  }

  // write the minute
  if (tm_last.tm_min != tm_now->tm_min) {
    tft.fillRect(TIME_LEFT+TIME_SPACE, TIME_TOP, TIME_SPACE, TIME_HEIGHT, COL_BACK);
    tft.setTextColor(COL_FORE);
    tft.setCursor(TIME_LEFT+TIME_SPACE, TIME_TOP);
    tft.printf("%02d:", tm_now->tm_min);
  }

  // write the second
  if (tm_last.tm_sec != tm_now->tm_sec) {
    tft.fillRect(TIME_LEFT+2*TIME_SPACE, TIME_TOP, TIME_SPACE, TIME_HEIGHT, COL_BACK);
    tft.setTextColor(COL_FORE);
    tft.setCursor(TIME_LEFT+2*TIME_SPACE, TIME_TOP);
    tft.printf("%02d", tm_now->tm_sec);
  }

  // Print the date on the next line
  if (tm_last.tm_mday != tm_now->tm_mday || tm_last.tm_mon != tm_now->tm_mon || tm_last.tm_year != tm_now->tm_year) {
    tft.fillRect(0, DATE_BASE-DATE_HEIGHT+2, SCREEN_WIDTH-1, DATE_HEIGHT, COL_BACK);

    #define LEN_BUF 256
    char buf[LEN_BUF];
    strftime(buf, LEN_BUF, "%a, %d %b %Y", tm_now);

    tft.setFreeFont(DATE_FONT);
    tft.setTextDatum(BC_DATUM);
    tft.drawString(buf, SCREEN_WIDTH/2, DATE_BASE);
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
  // Temperature
  tft.fillRect(TEMP_LEFT, TEMP_BASE-TEMP_HEIGHT+4, TEMP_WIDTH, TEMP_HEIGHT, COL_BACK);
  tft.setFreeFont(TEMP_FONT);
  tft.setCursor(TEMP_LEFT, TEMP_BASE);
  tft.printf("%02d C", Temperature);
  tft.setFreeFont(FSS9);
  tft.setCursor(TEMP_LEFT+55, TEMP_BASE-25);
  tft.print("o");

  // Humidity and pressure
  tft.fillRect(HUMID_LEFT, HUMID_BASE-2*HUMID_HEIGHT+4, SCREEN_WIDTH-HUMID_LEFT, 2*HUMID_HEIGHT, COL_BACK);
  tft.setFreeFont(HUMID_FONT);

  tft.setCursor(HUMID_LEFT, HUMID_BASE-HUMID_HEIGHT);
  tft.print("Humidity");
  tft.setCursor(HUMID_LEFT, HUMID_BASE);
  tft.print("Pressure");

  tft.setCursor(HUMID_LEFT+HUMID_SPACE, HUMID_BASE-HUMID_HEIGHT);
  tft.printf("%d%%", Humidity);
  tft.setCursor(HUMID_LEFT+HUMID_SPACE, HUMID_BASE);
  tft.printf("%d", Pressure);
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

  // Start the tft display and set it to black
  tft.init();
  // My display needs the colours inverted for some reason
  tft.invertDisplay(true);
  // This is the display in landscape with the origin at top left
  tft.setRotation(3);
  // Clear the screen before writing to it
  tft.fillScreen(COL_BACK);
  DisplayStatus("Starting clock ...");

  // Start the task to get the time
  TaskHandle_t h;
  xTaskCreate(UpdateTime, "UpdateTime", 2000, NULL, 2, &h);

  // Start the SPI for the touchscreen and init the touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
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