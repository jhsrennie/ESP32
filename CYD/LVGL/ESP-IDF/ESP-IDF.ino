//----------------------------------------------------------------------
// ESP-IDF
// =======
// This demonstrates how to use the LVGL library with the ESP-IDF LCD
// functions. Doing this means you do not need to use TFT_eSPI.
//----------------------------------------------------------------------
#include <driver/spi_common.h>
// This includes the stuff needed for SPI displays
#include <esp_lcd_io_spi.h>
// This includes the basic functions like init and reset
#include <esp_lcd_panel_ops.h>
// These two includes contain the stuff needed for the ST7789 (ILI9341)
// controller used in the CYD
#include <esp_lcd_panel_dev.h>
#include <esp_lcd_panel_st7789.h>
// LVGL library
#include <lvgl.h>

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
// LVGL prototypes
// ---------------
// These functions are defined in LVGLStuff.cpp
//----------------------------------------------------------------------
bool InitLVGL(void* panel_handle, int32_t hres, int32_t vres, lv_display_flush_cb_t flush_cb);
void CreateExampleScreen();

//----------------------------------------------------------------------
// InitST7789
// ----------
// There's quite a lot to do to get the LCD initialised, but it's all
// fairly straightforward.
// The mirroring and swap options chosen will put the origin at the top
// left if you have the USB port at the left.
//----------------------------------------------------------------------
bool InitST7789() {
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
  ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &ScrPanel));

  // Reset the display
  ESP_ERROR_CHECK(esp_lcd_panel_reset(ScrPanel));
  // Initialize LCD panel
  ESP_ERROR_CHECK(esp_lcd_panel_init(ScrPanel));
  // Turn on the screen
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(ScrPanel, true));
  // On my CYD the colours are not inverted
  ESP_ERROR_CHECK(esp_lcd_panel_invert_color(ScrPanel, false));
  // On my CYD the x and y axes are swapped
  ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(ScrPanel, true));
  // On my CYD the screen y axis is mirrored
  ESP_ERROR_CHECK(esp_lcd_panel_mirror(ScrPanel, false, true));

  // Return indicating success
  return true;
}

//----------------------------------------------------------------------
// BlitST7789
// ----------
// Function used by LVGL to write the buffer to the screen
// This is the only function we need to write to get LVGL working. We
// tell LVGL to use this function using lv_display_set_flush_cb().
//----------------------------------------------------------------------
static void BlitST7789(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map)
{
  esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) lv_display_get_user_data(disp);
  // because SPI LCD is big-endian, we need to swap the RGB bytes order
  lv_draw_sw_rgb565_swap(px_map, (area->x2 + 1 - area->x1) * (area->y2 + 1 - area->y1));
  // Blit the data to the display
  esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map);
  // We need to wait a few milliseconds for the DMA to complete
  // The delay was determined by trial and error. Really we should have a
  // DMA completion routine - oh well.
  delay(8);
  // Notify LVGL that the blit has been done
  lv_display_flush_ready(disp);
}

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("LVGL-IDF started");

  // Initialise the display controller
  if (!InitST7789())
    vTaskSuspend(NULL);

  // Turn the backlight on
  pinMode(PIN_NUM_BK_LIGHT, OUTPUT);
  digitalWrite(PIN_NUM_BK_LIGHT, HIGH);

  // Initialise LVGL
  if (!InitLVGL((void*) ScrPanel, LCD_H_RES, LCD_V_RES, BlitST7789)) {
    Serial.println("LVGLInit() failed");
    vTaskSuspend(NULL);
  }

  // Create example screen
  CreateExampleScreen();
}

//----------------------------------------------------------------------
// loop
// ----
//----------------------------------------------------------------------
void loop() {
  lv_timer_handler();
  lv_tick_inc(5);
  delay(5);
}