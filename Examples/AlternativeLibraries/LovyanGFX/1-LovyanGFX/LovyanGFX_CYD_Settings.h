//----------------------------------------------------------------------
// LovyanGFX_CYD_Settings.h
// ========================
// This is a Lovyan settings file that works with my CYD board.
// I have heavily edited down the original settings file to clear out
// lots of unnecessary comments (that were in Japanese anyway).
//----------------------------------------------------------------------

class LGFX : public lgfx::LGFX_Device
{
  // Type of panel you want to connect - the CYD uses an ILI9341
  lgfx::Panel_ILI9341 _panel_instance;

  // Type of bus - the CYD uses HSPI for the display
  lgfx::Bus_SPI _bus_instance;

  // Enable backlight control - pin 21 on the CYD
  lgfx::Light_PWM _light_instance;

  // Type of touchscreen type - the CYD uses an XPT2046
  lgfx::Touch_XPT2046 _touch_instance;

public:

  // Constructor
  LGFX(void)
  {
    // Configure the display SPI bus settings
    {
      auto cfg = _bus_instance.config();

      cfg.spi_host = HSPI_HOST; // The CYD uses HSPI for the display
      cfg.spi_mode = 0; // Don't know what this does
      // I found the following settings in the Internet. I don't know if they are optimal.
      cfg.freq_write = 55000000;
      cfg.freq_read  = 16000000;
      cfg.spi_3wire  = false; // Set to true if receiving is done on the MOSI pin
      cfg.use_lock   = true;  // Set to true if you want to use transaction locking
      // With the ESP-IDF version upgrade, SPI_DMA_CH_AUTO (automatic setting) is now recommended for the DMA channel. Specifying 1ch or 2ch is no longer recommended.
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      // HSPI GPIO pins on the CYD
      cfg.pin_sclk = 14;
      cfg.pin_mosi = 13;
      cfg.pin_miso = 12;
      cfg.pin_dc = 2;

      // Set the config
      _bus_instance.config(cfg);
      // Place the bus on the panel
      _panel_instance.setBus(&_bus_instance);
    }

    // Configure the display panel control settings
    {
      auto cfg = _panel_instance.config();

      cfg.pin_cs = 15;
      cfg.pin_rst = -1; // -1 = disable
      cfg.pin_busy = -1; // -1 = disable

      // I had to change the usual ILI9341 dimensions
      // cfg.panel_width = 240;
      // cfg.panel_height = 320;
      cfg.panel_width = 320;
      cfg.panel_height = 240;
      cfg.offset_x         =     0;
      cfg.offset_y         =     0;
      // On the CYD rotations 0 to 3 are mirrored so use 4 to 7
      cfg.offset_rotation  =     0;
      cfg.dummy_read_pixel =     8;  // Number of dummy read bits before pixel readout
      cfg.dummy_read_bits  =     1;  // Number of dummy read bits before reading non-pixel data
      cfg.readable         =  true;  // Set to true if data can be read
      cfg.invert           = false;  // Set to true if the panel's brightness is reversed
      // On my CYD red and blue were indeed swapped
      // cfg.rgb_order        = false;  // Set to true if the red and blue of the panel are swapped
      cfg.rgb_order        = true;  // Set to true if the red and blue of the panel are swapped
      cfg.dlen_16bit       = false;  // Set to true for panels that transmit data in 16-bit units via 16-bit parallel or SPI
      // If the bus is shared with the SD card, set it to true (bus control is performed using drawJpgFile, etc.)
      cfg.bus_shared       =  true;

      // These don't normally need changing but on the CYD I had to change them to get it to work
      // cfg.memory_width     =   240;
      // cfg.memory_height    =   320;
      cfg.memory_width     =   320;
      cfg.memory_height    =   240;

      // Set the config
      _panel_instance.config(cfg);
    }

    // Set the backlight control - comment out if not required
    {
      auto cfg = _light_instance.config();

      cfg.pin_bl = 21;
      cfg.invert = false;  // true if the backlight brightness is inverted
      cfg.freq   = 44100;  // Backlight PWM frequency
      cfg.pwm_channel = 7; // PWM channel number to use

      // Set the config
      _light_instance.config(cfg);
      // Place the backlight on the panel
      _panel_instance.setLight(&_light_instance);
    }

    // Configure touchscreen control - comment out if not needed
    {
      auto cfg = _touch_instance.config();

      // Touchscreen limits
      cfg.x_min = 0;
      cfg.x_max = 239;
      cfg.y_min = 0;
      cfg.y_max = 319;

      cfg.pin_int = -1; // Interrupt GPIO pin, -1 = disable
      cfg.bus_shared = true; // If you are using a bus that is common with the screen, set it to true
      cfg.offset_rotation = 0; // Adjust if display and touch direction do not match. Set a value between 0 and 7.

      cfg.spi_host = VSPI_HOST; // The CYD uses VSPI for the touchscreen
      cfg.freq = 1000000;

      // VSPI GPIO pins on the CYD
      cfg.pin_sclk = 25;
      cfg.pin_mosi = 32;
      cfg.pin_miso = 39;
      cfg.pin_cs   = 33;

      // Set the config
      _touch_instance.config(cfg);
      // Place the touchscreen on the panel
      _panel_instance.setTouch(&_touch_instance);
    }

    // Apply all the above settings to the panel
    setPanel(&_panel_instance);
  }
};