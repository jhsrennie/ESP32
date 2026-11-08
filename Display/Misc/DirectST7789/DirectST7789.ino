//----------------------------------------------------------------------
// DirectST7789
// ============
// This is example code to show how to directly control an ST7789
// display without using any library.
// You wouldn't do this in real life but it's interesting to see how the
// display works at the bare metal level.
//----------------------------------------------------------------------
#include "driver/spi_master.h"

// ST7789 pins
#define PIN_MISO      -1 // Not used
#define PIN_MOSI       2
#define PIN_SCLK      15
#define PIN_DC        16
#define PIN_CS        -1 // The board I am testing has no CS pin
#define PIN_RST        4
#define PIN_BCKL      17

// Use the HSPI bus for the display
#define LCD_HOST SPI2_HOST

// Size of the CYD display
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240

// ST7789 Commands used in this example
uint8_t CMD_SWRESET = 0x01;
uint8_t CMD_SLPOUT = 0x11;
uint8_t CMD_INVON  = 0x21;
uint8_t CMD_COLMOD = 0x3A;
uint8_t CMD_MADCTL = 0x36;
uint8_t CMD_DISPON = 0x29;
uint8_t CMD_CASET = 0x2A;
uint8_t CMD_RASET = 0x2B;
uint8_t CMD_RAMWR = 0x2C;

// Use a global for the SPI handle
spi_device_handle_t spiHandle;

// The buffer used for display data
// Typically this is 10% of the screen area with 2 bytes per pixel
#define DRAW_BUF_LINES  (SCREEN_HEIGHT/10)
#define DRAW_BUF_PIXELS (SCREEN_WIDTH*DRAW_BUF_LINES)
#define DRAW_BUF_SIZE   (2*DRAW_BUF_PIXELS)
uint8_t* drawBuf = NULL;

//----------------------------------------------------------------------
// lcd_send_cmd
// ------------
// Send a command to the LCD
//----------------------------------------------------------------------
void lcd_send_cmd(spi_device_handle_t spiHandle, const uint8_t* cmd, size_t cmd_size, const uint8_t* param, size_t param_size) {
  // Send the command byte
  if (cmd && cmd_size > 0) {
    spi_transaction_t t = {0};
    t.length    = cmd_size*8; // Length in bits
    t.tx_buffer = cmd;        // Data to send
    t.user      = (void*) 0;  // D/C line LOW for a command
    // esp_err_t e = spi_device_polling_transmit(spiHandle, &t);
    esp_err_t e = spi_device_transmit(spiHandle, &t);
    if (e != ESP_OK) {
      Serial.printf("Command spi_device_polling_transmit() returned %d\n", e);
      return;
    }
  }

  // Send the parameters
  if (param && param_size > 0) {
    spi_transaction_t t = {0};
    t.length    = param_size*8; // Length in bits
    t.tx_buffer = param;        // Data to send
    t.user = (void*) 1;         // D/C line HIGH for data
    // esp_err_t e = spi_device_polling_transmit(spiHandle, &t);
    esp_err_t e = spi_device_transmit(spiHandle, &t);
    if (e != ESP_OK) {
      Serial.printf("Parameter spi_device_polling_transmit() returned %d\n", e);
      return;
    }
  }
}

//----------------------------------------------------------------------
// spi_pre_transfer_callback
// -------------------------
// This called whenever we do an SPI transaction. It sets the DC pin to
// the value set in the spi_transaction_t.user field.
// We use this to set DC low for a command and high for data.
//----------------------------------------------------------------------
void spi_pre_transfer_callback(spi_transaction_t *t) {
  int dc = (int)t->user;
  gpio_set_level((gpio_num_t) PIN_DC, dc);
}

//----------------------------------------------------------------------
// spi_init
// --------
// Initialise the SPI bus
//----------------------------------------------------------------------
spi_device_handle_t spi_init() {
  esp_err_t ret;
  spi_device_handle_t spi;

  // Configure the GPIOs we need
  gpio_config_t io_conf = {0};
  if (PIN_RST == -1)
    io_conf.pin_bit_mask = ((1ULL << PIN_DC) | (1ULL << PIN_BCKL));
  else
    io_conf.pin_bit_mask = ((1ULL << PIN_DC) | (1ULL << PIN_RST) | (1ULL << PIN_BCKL));
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  gpio_config(&io_conf);

  // Configure the SPI Bus
  spi_bus_config_t buscfg;
  memset(&buscfg, 0, sizeof(buscfg));
  buscfg.miso_io_num = PIN_MISO;
  buscfg.mosi_io_num = PIN_MOSI;
  buscfg.sclk_io_num = PIN_SCLK;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;
  buscfg.max_transfer_sz = DRAW_BUF_SIZE;
  esp_err_t e = spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
  if (e != ESP_OK) {
    Serial.printf("spi_bus_initialize() returned %d\n", e);
    return 0;
  }

  // Configure the display device
  // My 240x240 ST7789 needs the SPI mode set to 3 instead of the usual
  // value of 0.
  spi_device_interface_config_t devcfg;
  memset(&devcfg, 0, sizeof(devcfg));
  devcfg.clock_speed_hz = 26000000; // SPI Clock speed (26 MHz)
  devcfg.mode           = 3;
  devcfg.spics_io_num   = PIN_CS;   // CS GPIO pin
  devcfg.queue_size     = 7;        // Queue transaction limit
  devcfg.pre_cb         = spi_pre_transfer_callback; // Auto-handle D/C pin toggling

  e = spi_bus_add_device(LCD_HOST, &devcfg, &spi);
  if (e != ESP_OK) {
    Serial.printf("spi_bus_add_device() returned %d\n", e);
    return 0;
  }

  // Finally allocate the buffer for display data
  drawBuf = (uint8_t*) heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_DMA);
  if (!drawBuf) {
    Serial.println("Draw buffer allocation failed");
    return 0;
  }

  // Return indicating success
  return spi;
}

//----------------------------------------------------------------------
// lcd_init
// --------
//----------------------------------------------------------------------
void lcd_init(spi_device_handle_t spi) {
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

  // Reset: wait 5ms if sleep mode is off, 120ms if sleep mode is on
  lcd_send_cmd(spi, &CMD_SWRESET, 1, NULL, 0);
  vTaskDelay(pdMS_TO_TICKS(120));

  // Turn off sleep mode: wait 5ms if sleep mode is off, 120ms if sleep mode is on
  lcd_send_cmd(spi, &CMD_SLPOUT, 1, NULL, 0);
  vTaskDelay(pdMS_TO_TICKS(120));

  // My ST7789 needs inversion on: no wait needed
  lcd_send_cmd(spi, &CMD_INVON, 1, NULL, 0);

  // Color mode: 16-bit color (RGB565): no wait needed
  uint8_t colmod_data = 0x05;
  lcd_send_cmd(spi, &CMD_COLMOD, 1, &colmod_data, 1);

  // Memory Access Control: no wait needed
  uint8_t madctl_data = 0x00; 
  lcd_send_cmd(spi, &CMD_MADCTL, 1, &madctl_data, 1);

  // Display on: no wait needed
  lcd_send_cmd(spi, &CMD_DISPON, 1, NULL, 0);
}

//----------------------------------------------------------------------
// lcd_set_window
// --------------
// Set drawing window coordinates on display memory boundary
//----------------------------------------------------------------------
void lcd_set_window(spi_device_handle_t spi, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
  uint8_t data[4];

  data[0] = (x1 >> 8) & 0xFF;
  data[1] = x1 & 0xFF;
  data[2] = (x2 >> 8) & 0xFF;
  data[3] = x2 & 0xFF;
  lcd_send_cmd(spi, &CMD_CASET, 1, data, 4);

  data[0] = (y1 >> 8) & 0xFF;
  data[1] = y1 & 0xFF;
  data[2] = (y2 >> 8) & 0xFF;
  data[3] = y2 & 0xFF;
  lcd_send_cmd(spi, &CMD_RASET, 1, data, 4);
}

//----------------------------------------------------------------------
// lcd_fill
// --------
// Fill the display with the specified colour
//----------------------------------------------------------------------
void lcd_fill(spi_device_handle_t spi, uint16_t col) {
  // Set the window to the whole screen
  lcd_set_window(spi, 0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1);

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
// setup
// -----
//----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Starting DirectST7789");

  // Initialise the SPI bus
  spiHandle = spi_init();
  Serial.println("SPI initialised");

  // Initialise the display
  lcd_init(spiHandle);
  Serial.println("LCD initialised");
}

//----------------------------------------------------------------------
// loop
// ----
//----------------------------------------------------------------------
void loop() {
  uint16_t red_pixel = 0xF800;
  lcd_fill(spiHandle, red_pixel);
  delay(1000);

  uint16_t green_pixel = 0x07E0;
  lcd_fill(spiHandle, green_pixel);
  delay(1000);

  uint16_t blue_pixel = 0x001F;
  lcd_fill(spiHandle, blue_pixel);
  delay(5000);
}