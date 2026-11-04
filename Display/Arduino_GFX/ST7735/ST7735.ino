//----------------------------------------------------------------------
// ST7735
// ------
// Example sketch to show how to use the ST7735 display on an ESP32
// using the Arduino GFX library.
//----------------------------------------------------------------------
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// Define the pins used
#define PIN_MOSI       2
#define PIN_MISO      -1 // The ST7735 does not use MISO
#define PIN_SCK       15
#define PIN_DC        16
#define PIN_CS        17
#define PIN_RST        4
#define PIN_BACKLIGHT  5

// Create the display object
Adafruit_ST7735 tft(&SPI, PIN_CS, PIN_DC, PIN_RST);

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup(void) {
  // Initialise the serial monitor and wait for it to complete
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting ST7735 with the Arduino_GFX library");

  // Turn on the backlight
  pinMode(PIN_BACKLIGHT, OUTPUT);
  digitalWrite(PIN_BACKLIGHT, HIGH);

  // Configure the SPI bus to use our selected pins
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);

  // Uncomment the init call for your size screen
  // 0.96" 160x80 TFT:
  tft.initR(INITR_MINI160x80);  // Init ST7735S mini display

  // 0.96" 160x80 TFT with plug-in FPC (if you see the display is inverted!)
  // tft.initR(INITR_MINI160x80_PLUGIN);  // Init ST7735S mini display

  // 1.44" TFT screen:
  // tft.initR(INITR_144GREENTAB); // Init ST7735R chip, green tab

  // 1.8" TFT screen:
  // tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab

  // 1.8" TFT screen with offset such as WaveShare:
  // tft.initR(INITR_GREENTAB);      // Init ST7735S chip, green tab

  Serial.println("Initialised");
}

//----------------------------------------------------------------------
// loop
// ----
//----------------------------------------------------------------------
void loop() {
  tft.fillScreen(ST77XX_RED);
  delay(1000);
  tft.fillScreen(ST77XX_GREEN);
  delay(1000);
  tft.fillScreen(ST77XX_BLUE);
  delay(5000);
}