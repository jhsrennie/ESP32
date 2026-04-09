//----------------------------------------------------------------------
// ILI9341-Driver
// ==============
// An attempt to code the ILI9341 controller from scratch
//----------------------------------------------------------------------
#include <driver/spi_common.h>
// This includes the stuff needed for SPI displays
#include <esp_lcd_io_spi.h>
#include <esp_lcd_panel_dev.h>
#include "esp_lcd_ili9341.h"

// The CYD uses SPI2 (HSPI) for the display
#define LCD_HOST SPI2_HOST

// Pin definitions etc
#define LCD_PIXEL_CLOCK_HZ     (20*1000*1000)
#define PIN_NUM_MOSI           13
#define PIN_NUM_MISO           12
#define PIN_NUM_SCLK           14
#define PIN_NUM_CS             15
#define PIN_NUM_DC             2
#define PIN_NUM_RST            3
#define PIN_NUM_BK_LIGHT       21

// Horizontal and vertical resolution
#define LCD_H_RES              320
#define LCD_V_RES              240

// Bit number used to represent command and parameter
#define LCD_CMD_BITS           8
#define LCD_PARAM_BITS         8

// Display buffer size
#define BUF_DEPTH              24 // Covers 24 lines of the display
#define BUF_LEN                (LCD_H_RES*BUF_DEPTH)
uint16_t* ScrBuf = NULL;

// Display panel handle
esp_lcd_panel_handle_t ScrPanel = NULL;

//----------------------------------------------------------------------
// RGBCol
// ------
// Return a 16 bit value to set the colour on the ST7790 where:
// red = top 5 bits
// green = middle 6 bits
// blue = bottom 5 bits
// But the colours need to be in big endian format. Since the CYD uses
// little endian format the two bytes need to be swapped.
//----------------------------------------------------------------------
#define BYTE_SWAP(v) ((v >> 8) | (v << 8))
uint16_t RGBCol(uint8_t Red, uint8_t Green, uint8_t Blue) {
  // Construct the 16 bit colour rrrrrggggggbbbbb
  uint16_t col = ((Red >> 3) << 11) | ((Green >> 2) << 5) | (Blue >> 3);
  // Swap the bytes to make it big endian
  col = BYTE_SWAP(col);
  // Return the colour
  return col;
}

//----------------------------------------------------------------------
// InitDisplay
// -----------
// There's quite a lot to do to get the LCD initialised, but it's all
// fairly straightforward.
// The mirroring and swap options chosen will put the origin at the top
// left if you have the USB port at the left.
//----------------------------------------------------------------------
bool InitDisplay() {
  // Initialize the SPI bus
  spi_bus_config_t buscfg = {0};
  buscfg.sclk_io_num = PIN_NUM_SCLK;
  buscfg.mosi_io_num = PIN_NUM_MOSI;
  buscfg.miso_io_num = PIN_NUM_MISO;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;
  buscfg.max_transfer_sz = BUF_LEN*2 + 8;
  ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // Attach the LCD to the SPI bus
  esp_lcd_panel_io_handle_t io_handle = NULL;
  esp_lcd_panel_io_spi_config_t io_config = {0};
  io_config.dc_gpio_num = PIN_NUM_DC;
  io_config.cs_gpio_num = PIN_NUM_CS;
  io_config.pclk_hz = LCD_PIXEL_CLOCK_HZ;
  io_config.lcd_cmd_bits = LCD_CMD_BITS;
  io_config.lcd_param_bits = LCD_PARAM_BITS;
  io_config.spi_mode = 0;
  io_config.trans_queue_depth = 10;
  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

  // Initialize the LCD configuration
  // ESP-IDF has built in support for the ST7789/ILI9341. Just call
  // the esp_lcd_new_panel_st7789() function.
  esp_lcd_panel_dev_config_t panel_config = {0};
  panel_config.reset_gpio_num = PIN_NUM_RST;
  panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
  panel_config.bits_per_pixel = 16;
  ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(io_handle, &panel_config, &ScrPanel));

  // Reset the display
  ESP_ERROR_CHECK(panel_ili9341_reset(ScrPanel));
  // Initialize LCD panel
  ESP_ERROR_CHECK(panel_ili9341_init(ScrPanel));
  // Turn on the screen
  ESP_ERROR_CHECK(panel_ili9341_disp_on_off(ScrPanel, true));
  // On my CYD the colours are not inverted
  ESP_ERROR_CHECK(panel_ili9341_invert_color(ScrPanel, false));
  // On my CYD the x and y axes are not swapped
  ESP_ERROR_CHECK(panel_ili9341_swap_xy(ScrPanel, false));
  // On my CYD the screen x axis is mirrored
  ESP_ERROR_CHECK(panel_ili9341_mirror(ScrPanel, true, false));

  // Return indicating success
  return true;
}

//----------------------------------------------------------------------
// ClearScreen
// -----------
// Clear the screen by writing black to it
//----------------------------------------------------------------------
void ClearScreen() {
  // Fill the buffer with zeros
  for (int i = 0; i < BUF_LEN; i++)
    ScrBuf[i] = 0;
  // Blit the buffer enough times to cover the display
  for (int i = 0; i < LCD_H_RES/BUF_DEPTH; i++) {
    panel_ili9341_draw_bitmap(ScrPanel, 0, i*BUF_DEPTH, LCD_H_RES, (i+1)*BUF_DEPTH, ScrBuf);
    // For some reason that I don't understand we need at least a 3ms
    // delay or there is screen corruption. Maybe the DMA takes time to
    // complete?
    delay(10);
  }
}

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("ILI9341 starting");

  // Create the screen buffer
  ScrBuf = (uint16_t*) heap_caps_malloc(BUF_LEN*sizeof(uint16_t), MALLOC_CAP_DMA);
  if (!ScrBuf) {
    Serial.println("Memory allocation failed");
    vTaskSuspend(NULL);
  }

  // Initialise the screen
  if (!InitDisplay())
    vTaskSuspend(NULL);

  // Turn the backlight on
  pinMode(PIN_NUM_BK_LIGHT, OUTPUT);
  digitalWrite(PIN_NUM_BK_LIGHT, HIGH);

  // Clear the screen
  ClearScreen();

  // All done
  Serial.println("ILI9341 started");
}

void loop() {
  // Fill the buffer with a random colour
#define BLK_LEN (24*32)
  uint16_t colour = (uint16_t) (rand() % 0x10000);
  for (int i = 0; i < BLK_LEN; i++)
    ScrBuf[i] = colour;

  // Choose a random point on the screen
  int x = (rand() % 10)*32;
  int y = (rand() % 10)*24;
 
  // And blit the colour to the display
  panel_ili9341_draw_bitmap(ScrPanel,  x,  y, x+32, y+24, ScrBuf);
  delay(10);
}