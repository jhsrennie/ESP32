//----------------------------------------------------------------------
// ILI9341 240 x 320 display
// Cheap yellow display version.
// Copy this to your libraries/TFT_eSPI directory
//----------------------------------------------------------------------
#define USER_SETUP_INFO "User_Setup"

// My CYD board requires the "_2" driver. With the ILI9341_DRIVER it
// paints part of the screen
#define ILI9341_2_DRIVER
// And it requires colour inversion on
#define TFT_INVERSION_ON

#define TFT_WIDTH  240
#define TFT_HEIGHT 320

#define TFT_BL   21            // LED back-light control pin
#define TFT_BACKLIGHT_ON HIGH  // Level to turn ON back-light (HIGH or LOW)

#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1 // The CYD does not use the reset pin

#define TOUCH_CS 33     // Chip select pin (T_CS) of touch screen

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

#define SMOOTH_FONT

#define SPI_FREQUENCY  55000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
#define USE_HSPI_PORT // Use HSPI so touch can use VSPI
