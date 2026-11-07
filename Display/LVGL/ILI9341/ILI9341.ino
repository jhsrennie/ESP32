//----------------------------------------------------------------------
// ILI9341
// =======
// This is an attempt to use the built in ILI9341 driver provided by
// LVGL.
//----------------------------------------------------------------------
#include <lvgl.h>
#include <driver/spi_master.h>

// Screen size
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

// Buffer used by LVGL
#define DRAW_BUF_SIZE ((LV_COLOR_DEPTH/8)*SCREEN_WIDTH*SCREEN_HEIGHT/10)
uint8_t* drawBuf = NULL;

// If we are not using the TFT_eSPI or Lovyan drivers we need to
// configure the SPI bus
#define PIN_MISO      12
#define PIN_MOSI      13
#define PIN_SCK       14
#define PIN_DC         2
#define PIN_CS        15
#define PIN_RST       -1
#define PIN_BACKLIGHT 21

// Use the HSPI bus for the display
#define LCD_HOST SPI2_HOST

// SPI handle for the display
spi_device_handle_t spiHandle;

// LVGL text area
static lv_obj_t* txt;

//----------------------------------------------------------------------
// lcd_spi_pre_transfer_callback
// -----------------------------
// This called whenever we do an SPI transaction. It sets the DC pin to
// the value set in the spi_transaction_t.user field.
// We use this to set DC low for a command and high for data.
//----------------------------------------------------------------------
void lcd_spi_pre_transfer_callback(spi_transaction_t *t) {
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
  io_conf.pin_bit_mask = ((1ULL << PIN_DC) | (1ULL << PIN_BACKLIGHT));
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
  buscfg.sclk_io_num = PIN_SCK;
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
  devcfg.pre_cb         = lcd_spi_pre_transfer_callback; // Auto-handle D/C pin toggling
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
// my_lcd_send_cmd
// ---------------
// Send short command to the LCD. This function shall wait until the
// transaction finishes.
//----------------------------------------------------------------------
void my_lcd_send_cmd(lv_display_t *disp, const uint8_t* cmd, size_t cmd_size, const uint8_t* param, size_t param_size) {
  // Send the command byte
  if (cmd && cmd_size > 0) {
    spi_transaction_t t = {0};
    t.length    = cmd_size*8; // Length in bits
    t.tx_buffer = cmd;        // Data to send
    t.user      = (void*) 0;  // D/C line LOW for a command
    esp_err_t e = spi_device_polling_transmit(spiHandle, &t);
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
    esp_err_t e = spi_device_polling_transmit(spiHandle, &t);
    if (e != ESP_OK) {
      Serial.printf("Parameter spi_device_polling_transmit() returned %d\n", e);
      return;
    }
  }
}

//----------------------------------------------------------------------
// my_lcd_send_color
// -----------------
// Send large array of pixel data to the LCD. If necessary, this
// function has to do the byte-swapping.
// LVL recommends this send the data asynchronously as this improves
// performance. For testing I'm just using the my_lcd_send_cmd function.
//----------------------------------------------------------------------
void my_lcd_send_color(lv_display_t *disp, const uint8_t *cmd, size_t cmd_size, uint8_t *param, size_t param_size) {
  my_lcd_send_cmd(disp, cmd, cmd_size, param, param_size);
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
  lv_obj_set_size(txt, SCREEN_HEIGHT-10, SCREEN_WIDTH-10);
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

  // Create a display object
  lv_display_t * disp;
  // Common Flags
  // LV_LCD_FLAG_NONE: Default settings with no special orientation or color modifications.
  // LV_LCD_FLAG_MIRROR_X: Mirrors the display horizontally (column address order flip).
  // LV_LCD_FLAG_MIRROR_Y: Mirrors the display vertically (page address order flip).
  // LV_LCD_FLAG_BGR: Sets the color order to Blue-Green-Red instead of Red-Green-Blue (RGB).
  lv_lcd_flag_t flags = LV_LCD_FLAG_NONE;
  disp = lv_ili9341_create(SCREEN_WIDTH, SCREEN_HEIGHT, flags, my_lcd_send_cmd, my_lcd_send_color);
  lv_display_set_buffers(disp, drawBuf, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
  Serial.println("ILI9341 device created");

  // Function to draw the GUI
  CreateMainGUI();
  Serial.println("GUI created");

  // Finally turn the backlight on
  gpio_set_level((gpio_num_t) PIN_BACKLIGHT, 1);
}

//----------------------------------------------------------------------
// loop
// ----
//----------------------------------------------------------------------
void loop() {
  static int elapsed_time = 200;
  if (elapsed_time >= 200) {
    Serial.println("Foo!");
    elapsed_time = 0;
  }
  elapsed_time++;

  // If the user has typed anything into the serial monitor we will
  // append it to the edit control
  if (Serial.available()) {
    // Read a string from the serial monitor
    String s = Serial.readString();
    // Send the string to the display
    ProcessInput(s.c_str());
  }

  // Check the LVGL task handler
  // lv_task_handler();
  delay(5);
}