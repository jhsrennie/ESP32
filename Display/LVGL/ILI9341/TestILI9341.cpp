//----------------------------------------------------------------------
// TestILI9341
// ===========
// Functions to test the display
//----------------------------------------------------------------------
#include <driver/spi_master.h>
#include "driver/gpio.h"

// Prototype for SPI send command
void lcd_send_cmd(spi_device_handle_t spiHandle, const uint8_t* cmd, size_t cmd_size, const uint8_t* param, size_t param_size);

// Screen size
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

// CYD pins
// #define PIN_MISO      12
// #define PIN_MOSI      13
// #define PIN_SCK       14
// #define PIN_DC         2
// #define PIN_CS        15
// #define PIN_RST       -1
// #define PIN_BCKL      21

// Bare ILI9341 pins
#define PIN_MISO      15
#define PIN_MOSI      16
#define PIN_SCK        4
#define PIN_DC        17
#define PIN_CS        18
#define PIN_RST        5
#define PIN_BCKL       2

// The buffer used for display data
// Typically this is 10% of the screen area with 2 bytes per pixel
#define DRAW_BUF_LINES  (SCREEN_HEIGHT/10)
#define DRAW_BUF_PIXELS (SCREEN_WIDTH*DRAW_BUF_LINES)
#define DRAW_BUF_SIZE   (2*DRAW_BUF_PIXELS)
extern uint8_t* drawBuf;

// ILI9341 commands
uint8_t CMD_SWRESET = 0x01;
uint8_t CMD_SLPOUT  = 0x11;
uint8_t CMD_DISPON  = 0x29;
uint8_t CMD_CASET   = 0x2A;
uint8_t CMD_PASET   = 0x2B;
uint8_t CMD_RAMWR   = 0x2C;
uint8_t CMD_MADCTL  = 0x36;
uint8_t CMD_COLMOD  = 0x3A;

//----------------------------------------------------------------------
// lcd_init
// --------
//----------------------------------------------------------------------
static void lcd_init(spi_device_handle_t spi) {
  // Reset the screen
  if (PIN_RST != -1)
  {
    gpio_set_level((gpio_num_t) PIN_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level((gpio_num_t) PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  // Turn on backlight
  gpio_set_level((gpio_num_t) PIN_BCKL, 1);

  // Send Driver Initialization Sequence
  lcd_send_cmd(spi, &CMD_SWRESET, 1, NULL, 0);
  vTaskDelay(pdMS_TO_TICKS(150));
  lcd_send_cmd(spi, &CMD_SLPOUT, 1, NULL, 0);
  vTaskDelay(pdMS_TO_TICKS(150));

  // Interface Pixel Format: 16 bits/pixel (RGB565)
  uint8_t colmod_data = 0x55;
  lcd_send_cmd(spi, &CMD_COLMOD, 1, &colmod_data, 1);

  // Memory Access Control: Default standard orientation rotation layout
  uint8_t madctl_data = 0x08;
  lcd_send_cmd(spi, &CMD_MADCTL, 1, &madctl_data, 1);

  // Turn Display On
  lcd_send_cmd(spi, &CMD_DISPON, 1, NULL, 0);
  vTaskDelay(pdMS_TO_TICKS(100));
}

//----------------------------------------------------------------------
// lcd_fill
// --------
// Fill the display with the specified colour
//----------------------------------------------------------------------
static void lcd_fill(spi_device_handle_t spi, uint16_t col) {
  // Set the window to the whole screen
  uint8_t data[4];

  data[0] = data[1] = 0;
  data[2] = ((SCREEN_WIDTH-1) >> 8) & 0xFF;
  data[3] = (SCREEN_WIDTH-1) & 0xFF;
  lcd_send_cmd(spi, &CMD_CASET, 1, data, 4);

  data[2] = ((SCREEN_HEIGHT-1) >> 8) & 0xFF;
  data[3] = (SCREEN_HEIGHT-1) & 0xFF;
  lcd_send_cmd(spi, &CMD_PASET, 1, data, 4);

  // Solid red in RGB565 format: 11111 (R) 000000 (G) 00000 (B) -> 0xF800
  // Big-endian conversion for presentation on parallel bus stream:
  uint8_t color_buf[2];
  color_buf[0] = (col >> 8) & 0xFF;
  color_buf[1] = col & 0xFF;

  // Fill the draw buffer with the specified colour
  for (int i = 0; i < DRAW_BUF_PIXELS; i++) {
      drawBuf[i * 2] = color_buf[0];
      drawBuf[i * 2 + 1] = color_buf[1];
  }

  // Flush color array out to frame RAM
  lcd_send_cmd(spi, &CMD_RAMWR, 1, NULL, 0);

  int total_chunks = SCREEN_HEIGHT/DRAW_BUF_LINES;
  for (int chunk = 0; chunk < total_chunks; chunk++) {
    lcd_send_cmd(spi, NULL, 0, drawBuf, DRAW_BUF_SIZE);
  }
}

//----------------------------------------------------------------------
// TestILI9341
// -----------
//----------------------------------------------------------------------
void TestILI9341(spi_device_handle_t spi) {
  // Turn on backlight
  gpio_set_level((gpio_num_t) PIN_BCKL, 1);

  // Fill the screen
  lcd_init(spi);
  lcd_fill(spi, 0xF800);
  vTaskDelay(pdMS_TO_TICKS(1000));
  lcd_fill(spi, 0x07E0);
  vTaskDelay(pdMS_TO_TICKS(1000));
  lcd_fill(spi, 0x001F);
  vTaskDelay(pdMS_TO_TICKS(1000));

  // Reset the screen again
  if (PIN_RST != -1) {
    gpio_set_level((gpio_num_t) PIN_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level((gpio_num_t) PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}