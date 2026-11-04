//----------------------------------------------------------------------
// LovyanGFX_ST7735.h
// ==================
// This is a Lovyan settings file that works with the ST7735.
//----------------------------------------------------------------------

class LGFX : public lgfx::LGFX_Device
{
  // Type of panel you want to connect
  lgfx::Panel_ST7735 _panel_instance;

  // Type of bus
  lgfx::Bus_SPI _bus_instance;

  // Enable backlight control
  lgfx::Light_PWM _light_instance;

public:

  // Constructor
  LGFX(void)
  {
    // Configure the display SPI bus settings
    {
      auto cfg = _bus_instance.config();

      cfg.spi_host = HSPI_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 27000000;
      cfg.freq_read  = 16000000;
      cfg.spi_3wire  = false;
      cfg.use_lock   = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      // GPIO pins on my board
      cfg.pin_sclk = 15;
      cfg.pin_mosi =  2;
      cfg.pin_miso = -1;
      cfg.pin_dc   = 16;

      // Set the config
      _bus_instance.config(cfg);
      // Place the bus on the panel
      _panel_instance.setBus(&_bus_instance);
    }

    // Configure the display panel control settings
    {
      auto cfg = _panel_instance.config();

      cfg.pin_cs   = 17;
      cfg.pin_rst  =  4;
      cfg.pin_busy = -1; // -1 = disable

      cfg.panel_width  =  80;
      cfg.panel_height = 160;
      cfg.offset_x         =     0;
      cfg.offset_y         =     0;
      cfg.offset_rotation  =     0;
      cfg.dummy_read_pixel =     8;
      cfg.dummy_read_bits  =     1;
      cfg.readable         =  true;
      cfg.invert           = false;  // Set to true if the panel's brightness is reversed
      cfg.rgb_order        = false;  // Set to true if the red and blue of the panel are swapped
      cfg.dlen_16bit       = false;  // Set to true for panels that transmit data in 16-bit units via 16-bit parallel or SPI
      // If the bus is shared with the SD card, set it to true (bus control is performed using drawJpgFile, etc.)
      cfg.bus_shared       =  true;

      // cfg.memory_width     =    80;
      // cfg.memory_height    =   160;

      // Set the config
      _panel_instance.config(cfg);
    }

    // Set the backlight control - comment out if not required
    {
      auto cfg = _light_instance.config();

      cfg.pin_bl = 5;
      cfg.invert = false;  // true if the backlight brightness is inverted
      cfg.freq   = 44100;  // Backlight PWM frequency
      cfg.pwm_channel = 7; // PWM channel number to use

      // Set the config
      _light_instance.config(cfg);
      // Place the backlight on the panel
      _panel_instance.setLight(&_light_instance);
    }

    // Apply all the above settings to the panel
    setPanel(&_panel_instance);
  }
};