//----------------------------------------------------------------------
// ST7735 80 x 160 display
//----------------------------------------------------------------------
#define USER_SETUP_INFO "User_Setup"

#define ST7735_DRIVER

// There are different types of ST7735 display and you need to uncomment
// the correct option for your display.
// For the the correct option was ST7735_GREENTAB160x80 with inversion
// off, but you'll have to try the various options to find out what works
// for you.
// #define ST7735_INITB
// #define ST7735_GREENTAB
// #define ST7735_GREENTAB2
// #define ST7735_GREENTAB3
// #define ST7735_GREENTAB128    // For 128 x 128 display
#define ST7735_GREENTAB160x80 // For 160 x 80 display (BGR, inverted, 26 offset)
// #define ST7735_ROBOTLCD       // For some RobotLCD Arduino shields (128x160, BGR, https://docs.arduino.cc/retired/getting-started-guides/TFT)
// #define ST7735_REDTAB
// #define ST7735_BLACKTAB
// #define ST7735_REDTAB160x80   // For 160 x 80 display with 24 pixel offset
#define TFT_INVERSION_OFF

#define TFT_WIDTH   80
#define TFT_HEIGHT 160

#define TFT_BL    5            // LED back-light control pin
#define TFT_BACKLIGHT_ON HIGH  // Level to turn ON back-light (HIGH or LOW)

#define TFT_MISO -1
#define TFT_MOSI  2
#define TFT_SCLK 15
#define TFT_CS   17
#define TFT_DC   16
#define TFT_RST   4

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

#define SMOOTH_FONT

#define SPI_FREQUENCY      32000000
#define SPI_READ_FREQUENCY 16000000
#define USE_HSPI_PORT
