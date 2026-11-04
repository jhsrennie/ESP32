//----------------------------------------------------------------------
// ST7789
// ======
// Example sketch to show how to use the ST7789 display on an ESP32
// using the Arduino GFX library.
//----------------------------------------------------------------------
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// Define the pins used
#define PIN_MISO      -1 // Not used
#define PIN_MOSI       2
#define PIN_SCK       15
#define PIN_DC        16
#define PIN_CS        -1 // The board I am testing has no CS pin
#define PIN_RST        4
#define PIN_BACKLIGHT 17

// Create the display object
// Adafruit_ST7789 tft(&SPI, PIN_CS, PIN_DC, PIN_RST);
Adafruit_ST7789 tft(PIN_CS, PIN_DC, PIN_RST);

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup(void) {
  // Initialise the serial monitor and wait for it to complete
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting ST7789 with the Arduino_GFX library");

  // Turn on the backlight
  pinMode(PIN_BACKLIGHT, OUTPUT);
  digitalWrite(PIN_BACKLIGHT, HIGH);

  // Configure the SPI bus to use our selected pins
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);

  // My ST7789 board needs SPI_MODE3 to work
  tft.init(240, 240, SPI_MODE3);

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