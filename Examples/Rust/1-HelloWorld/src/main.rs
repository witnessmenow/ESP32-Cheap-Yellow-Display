use display_interface_spi::SPIInterfaceNoCS;
use embedded_graphics::{
    mono_font::{ascii::FONT_9X18, MonoTextStyle},
    pixelcolor::Rgb666,
    prelude::*,
    text::renderer::CharacterStyle,
    text::Text,
};

use esp_idf_hal::{
    delay::Ets,
    gpio,
    gpio::PinDriver,
    spi::{config::Config, config::DriverConfig, SpiDeviceDriver, SpiDriver, SPI2},
};

use mipidsi::Builder;

use std::error::Error;

use esp_idf_sys as _; // If using the `binstart` feature of `esp-idf-sys`, always keep this module imported

fn main() -> Result<(), Box<dyn Error>> {
    // It is necessary to call this function once. Otherwise some patches to the runtime
    // implemented by esp-idf-sys might not link properly. See https://github.com/esp-rs/esp-idf-template/issues/71
    esp_idf_sys::link_patches();

    // Reset, -1 or 4
    let rst = PinDriver::input_output_od(unsafe { gpio::Gpio4::new() })?;
    // Data Command control pin
    let dc = PinDriver::input_output_od(unsafe { gpio::Gpio2::new() })?;

    // Espressif built-in delay provider for small delays
    let mut delay = Ets;

    // Pin 14, Serial Clock
    let sclk = unsafe { gpio::Gpio14::new() };
    let spi = unsafe { SPI2::new() };
    // Pin 13, MOSI, Master Out Slave In
    let sdo = unsafe { gpio::Gpio13::new() };
    // Pin 12, MISO, Master In Slave Out
    let sdi = unsafe { gpio::Gpio12::new() };

    // Serial Peripheral Interface
    let spi = SpiDriver::new(spi, sclk, sdo, Some(sdi), &DriverConfig::new())?;

    // Pin 15, Chip select
    let cs = unsafe { gpio::Gpio15::new() };
    
    let spi = SpiDeviceDriver::new(spi, Some(cs), &Config::new())?;
    let di = SPIInterfaceNoCS::new(spi, dc);
    
    let mut display = Builder::ili9341_rgb666(di)
        .with_color_order(mipidsi::ColorOrder::Bgr)  // Colors are shifted, Blue -> Red
        .with_orientation(mipidsi::options::Orientation::Landscape(false))  // Mirror on text
        .init(&mut delay, Some(rst))
        .map_err(|_| Box::<dyn Error>::from("display init"))?;

    // Pin 21, Backlight
    let mut bl = PinDriver::output(unsafe { gpio::Gpio21::new() })?;
    // Turn on backlight
    bl.set_high()?;
    // Force the GPIO to hold it's high state and not go to sleep
    // TO check: why do we need this?
    unsafe { gpio_hold_en(21) };

    // fill the screen with black
    // TO check: this is quite slow somehow?
    display
         .clear(Rgb666::BLACK)
         .map_err(|_| Box::<dyn Error>::from("clear display"))?;

    // Create text style
    let mut style = MonoTextStyle::new(&FONT_9X18, Rgb666::WHITE);

    // Position x:5, y: 10
    Text::new("Hello", Point::new(5, 10), style)
        .draw(&mut display)
        .map_err(|_| Box::<dyn Error>::from("draw hello"))?;

    // Turn text to blue
    style.set_text_color(Some(Rgb666::BLUE));
    Text::new("World", Point::new(160, 26), style)
        .draw(&mut display)
        .map_err(|_| Box::<dyn Error>::from("draw world"))?;

    Ok(())
}
