//----------------------------------------------------------------------
// DirectILI9341
// =============
// This is example code to show how to directly control an ILI9341
// display without using any library.
// You wouldn't do this in real life but it's interesting to see how the
// display works at the bare metal level.
//----------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

// There are some differences between the cheap yellow display boards
// and other ILI9341s. The CYD has no reset pin and the colour order
// is RGB nor BGR. Uncomment this if you're using a CYD.
#define CYD 1

// This code was tested with a cheap yellow display (CYD) board.
// These are the pins the CYD uses for the display.
#define PIN_MISO   12
#define PIN_MOSI   13
#define PIN_CLK    14
#define PIN_CS     15
#define PIN_DC      2
#define PIN_RST    -1
#define PIN_BCKL   21

// The CYD uses the HSPI bus for the display
#define LCD_HOST SPI2_HOST

// Size of the CYD display
#define ILI9341_WIDTH  240
#define ILI9341_HEIGHT 320

// ILI9341 Commands used in this example
#define CMD_SWRESET    0x01
#define CMD_SLPOUT     0x11
#define CMD_DISPON     0x29
#define CMD_CASET      0x2A
#define CMD_PASET      0x2B
#define CMD_RAMWR      0x2C
#define CMD_MADCTL     0x36
#define CMD_COLMOD     0x3A

// Use a global for the SPI handle
spi_device_handle_t spi;

// The buffer used for display data
// Typically this is 10% of the screen area with 2 bytes per pixel
#define DRAW_BUF_LINES  (ILI9341_HEIGHT/10)
#define DRAW_BUF_PIXELS (ILI9341_WIDTH*ILI9341_HEIGHT/10)
#define DRAW_BUF_SIZE   (2*ILI9341_WIDTH*ILI9341_HEIGHT/10)
uint8_t* drawBuf = NULL;

//----------------------------------------------------------------------
// lcd_cmd
// -------
// Send a single command byte to the display
//----------------------------------------------------------------------
void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd) {
  // Serial.printf("Cmd: %02x\n", cmd);
  esp_err_t ret;
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length = 8;                         // Command is 8 bits
  t.tx_buffer = &cmd;                   // Data to send
  t.user = (void*)0;                    // D/C line LOW for command
  ret = spi_device_polling_transmit(spi, &t);
  assert(ret == ESP_OK);
}

//----------------------------------------------------------------------
// lcd_cmd
// -------
// Send data bytes to the display
//----------------------------------------------------------------------
void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len) {
  if (len == 0) return;
  // for (int i = 0; i < len; i++)
  //   Serial.printf("%02x ", data[i]);
  // Serial.println("");
  esp_err_t ret;
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));
  t.length = len * 8;                   // Length in bits
  t.tx_buffer = data;                   // Data to send
  t.user = (void*)1;                    // D/C line HIGH for data
  ret = spi_device_polling_transmit(spi, &t);
  assert(ret == ESP_OK);
}

//----------------------------------------------------------------------
// lcd_spi_pre_transfer_callback
// -----------------------------
// This called whenever we do an SPI transaction. It sets the DC pin to
// the value set in the spi_transaction_t.user field.
// We use this to set DC low for a command and high for data.
//----------------------------------------------------------------------
void lcd_spi_pre_transfer_callback(spi_transaction_t *t) {
  int dc = (int)t->user;
  gpio_set_level((gpio_num_t) PIN_DC, dc);
}

//----------------------------------------------------------------------
// init_ili9341_display
// --------------------
// Initialise the ILI9341 display
//----------------------------------------------------------------------
spi_device_handle_t init_ili9341_display() {
  esp_err_t ret;
  spi_device_handle_t spi;

  // Configure the GPIOs we need
  gpio_config_t io_conf = {0};
  // CYD has no reset pin
  #ifdef CYD
  io_conf.pin_bit_mask = ((1ULL << PIN_DC) | (1ULL << PIN_BCKL));
  #else
  io_conf.pin_bit_mask = ((1ULL << PIN_DC) | (1ULL << PIN_RST) | (1ULL << PIN_BCKL));
  #endif
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  gpio_config(&io_conf);

  // Reset the screen, though not if it's a CYD
  #ifndef CYD
  gpio_set_level(PIN_RST, 0);
  vTaskDelay(pdMS_TO_TICKS(100));
  gpio_set_level(PIN_RST, 1);
  vTaskDelay(pdMS_TO_TICKS(100));
  #endif

  // Turn on backlight
  gpio_set_level((gpio_num_t) PIN_BCKL, 1);

  // Configure the SPI Bus
  spi_bus_config_t buscfg;
  memset(&buscfg, 0, sizeof(buscfg));
  buscfg.miso_io_num = PIN_MISO;
  buscfg.mosi_io_num = PIN_MOSI;
  buscfg.sclk_io_num = PIN_CLK;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;
  buscfg.max_transfer_sz = DRAW_BUF_SIZE;
  esp_err_t e = spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
  if (e != ESP_OK) {
    Serial.printf("spi_bus_initialize() returned %d\n", e);
    return 0;
  }

  // Configure the display device
  spi_device_interface_config_t devcfg;
  memset(&devcfg, 0, sizeof(devcfg));
  devcfg.clock_speed_hz = 26000000; // SPI Clock speed (26 MHz)
  devcfg.mode           = 0;        // SPI mode 0
  devcfg.spics_io_num   = PIN_CS;   // CS GPIO pin
  devcfg.queue_size     = 7;        // Queue transaction limit
  devcfg.pre_cb         = lcd_spi_pre_transfer_callback; // Auto-handle D/C pin toggling

  e = spi_bus_add_device(LCD_HOST, &devcfg, &spi);
  if (e != ESP_OK) {
    Serial.printf("spi_bus_add_device() returned %d\n", e);
    return 0;
  }

  // Now we send a series of commands to initialise the ILI9341
  // Send Driver Initialization Sequence
  lcd_cmd(spi, CMD_SWRESET);
  vTaskDelay(pdMS_TO_TICKS(150));
  lcd_cmd(spi, CMD_SLPOUT);
  vTaskDelay(pdMS_TO_TICKS(150));

  // Interface Pixel Format: 16 bits/pixel (RGB565)
  uint8_t colmod_data = 0x55;
  lcd_cmd(spi, CMD_COLMOD);
  lcd_data(spi, &colmod_data, 1);

  // Memory Access Control: Default standard orientation rotation layout
  uint8_t madctl_data = 0x08;
  lcd_cmd(spi, CMD_MADCTL);
  lcd_data(spi, &madctl_data, 1);

  // Turn Display On
  lcd_cmd(spi, CMD_DISPON);
  vTaskDelay(pdMS_TO_TICKS(100));

  // Finally allocate the buffer for display data
  drawBuf = (uint8_t*) heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_DMA);
  if (!drawBuf) {
    Serial.println("Draw buffer allocation failed");
    return 0;
  }

  // Return indicating success
  Serial.println("ILI9341 initialised");
  return spi;
}

//----------------------------------------------------------------------
// Now we have the functions that control the display
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// lcd_set_window
// --------------
// Set drawing window coordinates on display memory boundary
//----------------------------------------------------------------------
void lcd_set_window(spi_device_handle_t spi, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
  uint8_t data[4];

  lcd_cmd(spi, CMD_CASET);
  data[0] = (x1 >> 8) & 0xFF;
  data[1] = x1 & 0xFF;
  data[2] = (x2 >> 8) & 0xFF;
  data[3] = x2 & 0xFF;
  lcd_data(spi, data, 4);

  lcd_cmd(spi, CMD_PASET);
  data[0] = (y1 >> 8) & 0xFF;
  data[1] = y1 & 0xFF;
  data[2] = (y2 >> 8) & 0xFF;
  data[3] = y2 & 0xFF;
  lcd_data(spi, data, 4);

  lcd_cmd(spi, CMD_RAMWR);
}

//----------------------------------------------------------------------
// lcd_fill
// --------
// Fill the display with the specified colour
//----------------------------------------------------------------------
void lcd_fill(spi_device_handle_t spi,   uint16_t col) {
  lcd_set_window(spi, 0, 0, ILI9341_WIDTH - 1, ILI9341_HEIGHT - 1);

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
  int total_chunks = ILI9341_HEIGHT/DRAW_BUF_LINES;
  for (int chunk = 0; chunk < total_chunks; chunk++) {
      lcd_data(spi, drawBuf, DRAW_BUF_SIZE);
  }
}

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Starting DirectILI9341");

  // Initialise the display
  spi = init_ili9341_display();
}

//----------------------------------------------------------------------
// loop
// ----
//----------------------------------------------------------------------
void loop() {
  uint16_t red_pixel = 0xF800;
  lcd_fill(spi, red_pixel);
  delay(1000);

  uint16_t green_pixel = 0x07E0;
  lcd_fill(spi, green_pixel);
  delay(1000);

  uint16_t blue_pixel = 0x001F;
  lcd_fill(spi, blue_pixel);
  delay(5000);
}