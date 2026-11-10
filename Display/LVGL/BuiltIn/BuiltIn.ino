//----------------------------------------------------------------------
// BuiltIn
// =======
// This is an attempt to use the built in ILI9341 driver provided by
// LVGL.
//----------------------------------------------------------------------
#include <lvgl.h>
#include <driver/spi_master.h>

// The version of the ILI9341 used in my cheap yellow display (CYD)
// board seems slightly diferent from a normal ILI9341
#define CYD

// Screen size
#ifdef CYD
// When using my CYD board I need to set width 320 x height 240
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#else
// But when using a standalone ILI9341 I need to set width 240 x height 320
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320
#endif

// Define the GPIO pins needed
#ifdef CYD
// CYD pins
#define PIN_MISO      12
#define PIN_MOSI      13
#define PIN_SCLK      14
#define PIN_DC         2
#define PIN_CS        15
#define PIN_RST       -1 // The CYD board doesn't have a reset pin
#define PIN_BCKL      21
#else
// Bare ILI9341 pins
#define PIN_MISO      15
#define PIN_MOSI      16
#define PIN_SCLK       4
#define PIN_DC        17
#define PIN_CS        18
#define PIN_RST       -1 // 5 for some reason this stops the display working if I set it
#define PIN_BCKL       2
#endif

// Use the HSPI bus for the display
#define LCD_HOST SPI2_HOST

// SPI handle for the display
spi_device_handle_t spiHandle;

// The buffer used for display data
// Typically this is 10% of the screen area with 2 bytes per pixel
#define DRAW_BUF_LINES  (SCREEN_HEIGHT/10)
#define DRAW_BUF_PIXELS (SCREEN_WIDTH*DRAW_BUF_LINES)
#define DRAW_BUF_SIZE   (2*DRAW_BUF_PIXELS)
uint8_t* drawBuf = NULL;

// LVGL text area
static lv_obj_t* txt;

//----------------------------------------------------------------------
// spi_pre_transfer_callback
// -------------------------
// This called whenever we do an SPI transaction. It sets the DC pin to
// the value set in the spi_transaction_t.user field.
// We use this to set DC low for a command and high for data.
//----------------------------------------------------------------------
void spi_pre_transfer_callback(spi_transaction_t *t) {
  int dc = (int) t->user;
  gpio_set_level((gpio_num_t) PIN_DC, dc);
}

//----------------------------------------------------------------------
// InitSPI
// -------
// Initialise the SPI bus
//----------------------------------------------------------------------
bool InitSPI() {
  // Configure the GPIOs we need
  gpio_config_t io_conf = {0};
  if (PIN_RST == -1) // Not all boards have a reset pin
    io_conf.pin_bit_mask = ((1ULL << PIN_DC) | (1ULL << PIN_BCKL));
  else
    io_conf.pin_bit_mask = ((1ULL << PIN_DC) | (1ULL << PIN_BCKL) | (1ULL << PIN_RST));
  io_conf.mode         = GPIO_MODE_OUTPUT;
  io_conf.pull_up_en   = GPIO_PULLUP_DISABLE;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.intr_type    = GPIO_INTR_DISABLE;
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
  spi_device_interface_config_t devcfg;
  memset(&devcfg, 0, sizeof(devcfg));
  devcfg.clock_speed_hz = 20000000; // SPI Clock speed (20 MHz)
  devcfg.mode           = 0;        // SPI mode 0
  devcfg.spics_io_num   = PIN_CS;   // CS GPIO pin
  devcfg.queue_size     = 7;        // Queue transaction limit
  devcfg.pre_cb         = spi_pre_transfer_callback; // Auto-handle D/C pin toggling
  e = spi_bus_add_device(LCD_HOST, &devcfg, &spiHandle);
  if (e != ESP_OK) {
    Serial.printf("spi_bus_add_device() returned %d\n", e);
    return 0;
  }

  // Allocate the buffer used by LVGL. Make it DMA capable.
  drawBuf = (uint8_t*) heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_DMA);

  // Return indicating success
  return true;
}

//----------------------------------------------------------------------
// If we are using the built in ILI9341 driver we need to provide
// functions to write data to the display.
//----------------------------------------------------------------------

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
      Serial.printf("Command transmit returned %d\n", e);
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
      Serial.printf("Parameter transmit returned %d\n", e);
      return;
    }
  }
}

//----------------------------------------------------------------------
// my_lcd_send_cmd
// ---------------
// Send short command to the LCD. This function shall wait until the
// transaction finishes.
//----------------------------------------------------------------------
void my_lcd_send_cmd(lv_display_t *disp, const uint8_t* cmd, size_t cmd_size, const uint8_t* param, size_t param_size) {
  // Blit the data
  lcd_send_cmd(spiHandle, cmd, cmd_size, param, param_size);

  // Notify LVGL that the blit has been done
  lv_display_flush_ready(disp);
}

//----------------------------------------------------------------------
// my_lcd_send_color
// -----------------
// Send large array of pixel data to the LCD. If necessary, this
// function has to do the byte-swapping.
// LVL recommends this send the data asynchronously as this improves
// performance. For testing I'm just using the lcd_send_cmd function.
//----------------------------------------------------------------------
void my_lcd_send_color(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, uint8_t *param, size_t param_size) {
  // Blit the data
  lcd_send_cmd(spiHandle, cmd, cmd_size, param, param_size);

  // Notify LVGL that the blit has been done
  lv_display_flush_ready(disp);
}

//----------------------------------------------------------------------
// log_print
// ---------
// If logging is enabled, it will inform the user about what is happening in the library
//----------------------------------------------------------------------
void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

//----------------------------------------------------------------------
// CreateMainGUI
// -------------
// Create the controls on the display
//----------------------------------------------------------------------
void CreateMainGUI(void) {
  // Create the text area
  txt = lv_textarea_create(lv_screen_active());
  // Resize the text area to fill the screen with a 5 pixel margin
  lv_obj_set_pos(txt, 5, 5);
  lv_obj_set_size(txt, SCREEN_WIDTH-10, SCREEN_HEIGHT-10);
  // Set the textarea event handler (currently unused)
  // lv_obj_add_event_cb(txt, textarea_event_handler, LV_EVENT_READY, txt);
  // Set the focus to the text area
  lv_obj_add_state(txt, LV_STATE_FOCUSED);
}

//----------------------------------------------------------------------
// ProcessInput
// ------------
// Add the string read from the serial monitor input to the edit control
//----------------------------------------------------------------------
void ProcessInput(const char* Input) {
  lv_textarea_add_text(txt, Input);
}

//----------------------------------------------------------------------
// LVGLTimerTask
// -------------
// This updates the LVGL timer every 5 milliseconds
//----------------------------------------------------------------------
void LVGLTimerTask(void* Parameter)
{
  while (true) {
    lv_tick_inc(5);     // tell LVGL how much time has passed
    delay(5);           // let this time pass
  }
}

//----------------------------------------------------------------------
// setup
// -----
//----------------------------------------------------------------------
void setup() {
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(LVGL_Arduino);

  // Initialise the SPI bus
  InitSPI();
  Serial.println("SPI bus initialised");

  // Start LVGL
  lv_init();
  // Register print function for debugging
  lv_log_register_print_cb(log_print);
  Serial.println("LVGL initialised");

  // Create the LVGL timer task before we create the display object
  TaskHandle_t h_timer;
  xTaskCreate(LVGLTimerTask, "LVGLTimerTask", 4000, NULL, 1, &h_timer);

  // Common Flags
  // LV_LCD_FLAG_NONE: Default settings with no special orientation or color modifications.
  // LV_LCD_FLAG_MIRROR_X: Mirrors the display horizontally (column address order flip).
  // LV_LCD_FLAG_MIRROR_Y: Mirrors the display vertically (page address order flip).
  // LV_LCD_FLAG_BGR: Sets the color order to Blue-Green-Red instead of Red-Green-Blue (RGB).
  #ifdef CYD
  lv_lcd_flag_t flags = LV_LCD_FLAG_MIRROR_X;
  #else
  lv_lcd_flag_t flags = LV_LCD_FLAG_NONE;
  #endif

  // Create a display object
  lv_display_t* disp;
  disp = lv_ili9341_create(SCREEN_WIDTH, SCREEN_HEIGHT, flags, my_lcd_send_cmd, my_lcd_send_color);
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_0);
  lv_display_set_buffers(disp, drawBuf, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
  Serial.println("ILI9341 device created");

  // Function to draw the GUI
  CreateMainGUI();
  Serial.println("GUI created");

  // Finally turn the backlight on
  gpio_set_level((gpio_num_t) PIN_BCKL, 1);
}

//----------------------------------------------------------------------
// loop
// ----
//----------------------------------------------------------------------
void loop() {
  // If the user has typed anything into the serial monitor we will
  // append it to the edit control
  if (Serial.available()) {
    // Read a string from the serial monitor
    String s = Serial.readString();
    // Send the string to the display
    ProcessInput(s.c_str());
  }

  // Check the LVGL task handler
  lv_task_handler();
  delay(5);
}