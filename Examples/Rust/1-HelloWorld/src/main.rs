use display_interface_spi::SPIInterfaceNoCS;
use embedded_graphics::{
    mono_font::{ascii::FONT_9X18, MonoTextStyle},
    pixelcolor::Rgb565,
    prelude::*,
    text::renderer::CharacterStyle,
    text::Text,
};

use esp_idf_svc::hal::{gpio, prelude::Peripherals};

use esp_idf_hal::{
    delay::Ets,
    spi::{config::{Config, DriverConfig}, Dma, SpiDeviceDriver}, units::MegaHertz,
};

use mipidsi::Builder;

use std::error::Error;

use esp_idf_sys as _; // If using the `binstart` feature of `esp-idf-sys`, always keep this module imported

fn main() -> Result<(), Box<dyn Error>> {
    // It is necessary to call this function once. Otherwise some patches to the runtime
    // implemented by esp-idf-sys might not link properly. See https://github.com/esp-rs/esp-idf-template/issues/71
    esp_idf_sys::link_patches();

    let peripherals = Peripherals::take().unwrap();
    let pins = peripherals.pins;

    // Reset, -1 or 4
    let rst = gpio::PinDriver::output(pins.gpio4)?;
    // Data Command control pin
    let dc = gpio::PinDriver::output(pins.gpio2)?;

    // Espressif built-in delay provider for small delays
    let mut delay = Ets;

    // Pin 14, Serial Clock
    let sclk = pins.gpio14;
    let spi = peripherals.spi2;
    // Pin 13, MOSI, Master Out Slave In
    let sdo = pins.gpio13;
    // Pin 12, MISO, Master In Slave Out
    let sdi = pins.gpio12;

    let cs = pins.gpio15;

    let di = SPIInterfaceNoCS::new(
        SpiDeviceDriver::new_single(
            spi,
            sclk,
            sdo,
            Some(sdi),
            Some(cs),
            &DriverConfig::new().dma(Dma::Disabled),
            &Config::new().baudrate(MegaHertz(40).into()),
        )?,
        dc,
    );

    let mut display = Builder::ili9341_rgb565(di)
        .with_color_order(mipidsi::ColorOrder::Rgb)
        .with_orientation(mipidsi::options::Orientation::LandscapeInverted(true))  // Mirror on text
        .init(&mut delay, Some(rst))
        .map_err(|_| Box::<dyn Error>::from("display init"))?;

    // Pin 21, Backlight
    let mut bl = gpio::PinDriver::output(pins.gpio21)?;
    // Turn on backlight
    bl.set_high()?;

    // Force the GPIO to hold it's high state
    core::mem::forget(bl);

    // fill the screen with black
    // TO check: this is quite slow somehow?
    display
         .clear(Rgb565::BLACK)
         .map_err(|_| Box::<dyn Error>::from("clear display"))?;

    // Create text style
    let mut style = MonoTextStyle::new(&FONT_9X18, Rgb565::WHITE);

    // Position x:5, y: 10
    Text::new("Hello", Point::new(5, 10), style)
        .draw(&mut display)
        .map_err(|_| Box::<dyn Error>::from("draw hello"))?;

    // Turn text to blue
    style.set_text_color(Some(Rgb565::BLUE));
    Text::new("World", Point::new(160, 26), style)
        .draw(&mut display)
        .map_err(|_| Box::<dyn Error>::from("draw world"))?;

    Ok(())
}