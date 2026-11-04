#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCK  14
#define TFT_DC    2
#define TFT_CS   15 
#define TFT_RST  -1

Adafruit_ILI9341 tft = Adafruit_ILI9341(&SPI, TFT_DC, TFT_CS, TFT_RST);

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Starting display test");

  // Turn on the backlight
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);

  // Start SPI
  SPI.begin(TFT_SCK, TFT_MISO, TFT_MOSI, TFT_CS);

  // Init display
  tft.begin();
}

void loop() {
  tft.fillScreen(ILI9341_RED);
  delay(1000);
  tft.fillScreen(ILI9341_GREEN);
  delay(1000);
  tft.fillScreen(ILI9341_BLUE);
  delay(5000);
}