//**********************************************************************
// ILI9341
// =======
// Example sketch to show how to use the ILI9341 display on an ESP32
// using the Arduino GFX library.
//**********************************************************************
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Screen size
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

// These are the pins my ILI9341 uses
#define PIN_MISO      15
#define PIN_MOSI      16
#define PIN_SCK        4
#define PIN_DC        17
#define PIN_CS        18
#define PIN_RST        5
#define PIN_BACKLIGHT  2

// The global SPI object uses the HSPI bus. To use VSPI create a new
// SPIClass object and pass VSPI to the constructor:
//   SPIClass myspi(VSPI);
//   Adafruit_ILI9341 tft = Adafruit_ILI9341(&myspi, PIN_DC, PIN_CS, PIN_RST);
//   myspi.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);

// Create the display object
Adafruit_ILI9341 tft = Adafruit_ILI9341(&SPI, PIN_DC, PIN_CS, PIN_RST);

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup() {
  // Initialise the serial monitor and wait for it to complete
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting ILI9341 CYD with the Arduino_GFX library");

  //We need to turn on the backlight as it is off by default
  pinMode(PIN_BACKLIGHT, OUTPUT);
  digitalWrite(PIN_BACKLIGHT, HIGH);

  // Configure the SPI bus to use our selected pins
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);

  // Init display
  tft.begin();
}

//----------------------------------------------------------------------
// loop
// ----
//----------------------------------------------------------------------
void loop() {
  // As a test cycle through various screen colours
  tft.fillScreen(ILI9341_RED);
  delay(1000);
  tft.fillScreen(ILI9341_GREEN);
  delay(1000);
  tft.fillScreen(ILI9341_BLUE);
  delay(5000);
}