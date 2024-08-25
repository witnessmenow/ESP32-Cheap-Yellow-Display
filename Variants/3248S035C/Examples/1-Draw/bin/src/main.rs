#![no_std]
#![no_main]
#![feature(type_alias_impl_trait)]

use core::cell::RefCell;
use critical_section::Mutex;
use display_interface_spi::SPIInterface;
use embassy_executor::Spawner;
use embassy_time::{Duration, Timer};
use embedded_hal_bus::spi::ExclusiveDevice;
use esp_backtrace as _;
use esp_hal::{
    clock::{
        ClockControl,
        Clocks,
    },
    delay::Delay,
    gpio::{
        GpioPin,
        Io,
        Level,
        Output,
    },
    peripherals::{
        Peripherals,
        SPI2,
        SPI3,
    },
    prelude::*,
    spi::{
        master::Spi,
        SpiMode,
    },
    system::SystemControl,
    timer::{
        ErasedTimer,
        OneShotTimer,
        timg::TimerGroup,
    },
};
use heapless::{
    Deque,
    Vec,
};
use mipidsi::{
    models::ILI9486Rgb565,
    options::{
        ColorOrder::Bgr,
        Orientation,
        Rotation,
    },
};

macro_rules! mk_static {
    ($t:ty,$val:expr) => {{
        static STATIC_CELL: static_cell::StaticCell<$t> = static_cell::StaticCell::new();
        #[deny(unused_attributes)]
        let x = STATIC_CELL.uninit().write(($val));
        x
    }};
}

#[main]
async fn main(spawner: Spawner) {
    esp_println::logger::init_logger_from_env();

    cfg_if::cfg_if! {
        if #[cfg(feature = "heap")] {
            heap::init_heap();
        }
    }

    let peripherals = Peripherals::take();

    // Configure clocks
    let system = SystemControl::new(peripherals.SYSTEM);
    let clocks = mk_static!(
        Clocks,
        ClockControl::max(system.clock_control).freeze());

    // IO pins
    let io = Io::new(peripherals.GPIO, peripherals.IO_MUX);
    let pins = io.pins;

    let led_red = pins.gpio4;
    let led_blue = pins.gpio16;
    let led_green = pins.gpio17;

    log::info!("RGB LEDs Off");
    let mut _led_red_inv = Output::new(led_red, Level::High);
    let mut _led_green_inv = Output::new(led_green, Level::High);
    let mut _led_blue_inv = Output::new(led_blue, Level::High);

    // embassy
    log::info!("Embassy");
    let timg1 = TimerGroup::new(peripherals.TIMG1, clocks, None);
    esp_hal_embassy::init(
        clocks,
        mk_static!([OneShotTimer<ErasedTimer>; 1], [OneShotTimer::new(timg1.timer0.into())])
    );

    spawner.spawn(touch_task(
        clocks,
        peripherals.SPI3,
        pins.gpio22, // sck,
        pins.gpio26, // mosi,
        pins.gpio39, // miso,
        pins.gpio33, // cs,
        pins.gpio36, // irq,
    )).unwrap();
    spawner.spawn(draw_task(
        clocks,
        peripherals.SPI2,
        pins.gpio14, // sck
        pins.gpio13, // mosi
        pins.gpio12, // miso
        pins.gpio15, // cs
        pins.gpio2, // dc
        pins.gpio27, // back_light
    )).unwrap();

    loop {
        Timer::after(Duration::from_millis(1_000)).await;
    }
}

// NOTE: embassy tasks cannot be generic, so we must double-specify the Peripheral instances
#[embassy_executor::task]
async fn touch_task(
    clocks: &'static Clocks<'_>,
    spi_instance: SPI3,
    pin_sck: GpioPin<22>,
    pin_mosi: GpioPin<26>,
    pin_miso: GpioPin<39>,
    pin_cs: GpioPin<33>,
    _pin_irq: GpioPin<36>,
) {
    log::info!("Touch Task");

    use touchscreen::{
        TouchEventType,
        TouchScreen,
        xpt2046::Xpt2046,
    };
    
    fn touch_calibration(raw: (u16, u16)) -> Option<(i32, i32)> {
        let (raw_x, raw_y) = raw;
        if raw_x < 4 || raw_y < 4 || raw_x > 4096-4 || raw_y > 4096-4 {
            return None;
        }
    
        let (x, y) = (
            raw_x as i32 * 480 / 4096,
            320 - (raw_y as i32 * 320 / 4096),
        );
    
        Some((x, y))
    }

    let touch_spi = 
        Spi::new(spi_instance, 2500.kHz(), SpiMode::Mode0, clocks)
            .with_sck(pin_sck)
            .with_mosi(pin_mosi)
            .with_miso(pin_miso);
    let touch_spi_device = ExclusiveDevice::new(
        touch_spi,
        Output::new(pin_cs, Level::High),
        Delay::new(clocks)).unwrap();
    let mut touch = Xpt2046::new(
        touch_spi_device,
        touch_calibration);

    loop {
        Timer::after(Duration::from_millis(10)).await;
        match touch.get_touch_event() {
            Ok(None) => (),
            Ok(Some(touch_event)) => {
                match touch_event.r#type {
                    TouchEventType::Start => {
                        critical_section::with(|cs| {
                            let mut deque = TOUCH.borrow_ref_mut(cs);
                            deque.clear();
                            let _ = deque.push_back((touch_event.x, touch_event.y));
                        });
                    },
                    TouchEventType::Move => {
                        critical_section::with(|cs| {
                            let mut deque = TOUCH.borrow_ref_mut(cs);
                            if let Err(point) = deque.push_back((touch_event.x, touch_event.y)) {
                                let _ = deque.pop_front();
                                let _ = deque.push_back(point);
                            };
                        });
                    },
                    TouchEventType::End => {
                        critical_section::with(|cs| {
                            TOUCH.borrow_ref_mut(cs).clear();
                        });
                    },
                }
            }
            Err(e) => {
                log::error!("TouchError: {e:?}");
            }
        }
    }
}

static TOUCH: Mutex<RefCell<Deque<(i32, i32), 32>>> = Mutex::new(RefCell::new(Deque::new()));

#[embassy_executor::task]
async fn draw_task(
    clocks: &'static Clocks<'_>,
    spi_instance: SPI2,
    pin_sck: GpioPin<14>,
    pin_mosi: GpioPin<13>,
    pin_miso: GpioPin<12>,
    pin_cs: GpioPin<15>,
    pin_dc: GpioPin<2>,
    pin_back_light: GpioPin<27>,
) {
    log::info!("LCD Task");

    use embedded_graphics::{
        pixelcolor::Rgb565,
        prelude::*,
        primitives::{
            Polyline,
            PrimitiveStyle,
        },
    };

    let lcd_spi = Spi::new(spi_instance, 80.MHz(), SpiMode::Mode0, clocks)
        .with_sck(pin_sck)
        .with_mosi(pin_mosi)
        .with_miso(pin_miso);
    let lcd_spi_device = ExclusiveDevice::new(
        lcd_spi,
        Output::new(pin_cs, Level::High),
        Delay::new(clocks)).unwrap();
    let di = SPIInterface::new(
        lcd_spi_device,
        Output::new(pin_dc, Level::High));
    let mut lcd = match mipidsi::Builder::new(ILI9486Rgb565, di)
        .color_order(Bgr)
        .display_size(320, 480)
        .orientation(Orientation {
            rotation: Rotation::Deg270,
            mirrored: true,
        })  // Mirror on text
        .init(&mut Delay::new(clocks))
    {
        Ok(ok) => ok,
        Err(err) => {
            panic!("Display error {err:?}");
        }
    };

    // Fill with black and then enable back-light
    lcd.clear(Rgb565::BLACK).unwrap();
    let mut _lcd_back_light = Output::new(pin_back_light, Level::High);

    loop {
        Timer::after(Duration::from_millis(10)).await;
        let deque = critical_section::with(|cs| {
            let deque = TOUCH.borrow_ref(cs);
            if deque.is_empty() {
                None
            } else {
                Some(deque.clone())
            }
        });
        match deque {
            None => lcd.clear(Rgb565::BLACK).unwrap(),
            Some(deque) => {
                let points = deque.iter()
                    .map(|(x, y)| Point::new(*x, *y))
                    .collect::<Vec<Point, 32>>();
                Polyline::new(&points)
                    .into_styled(PrimitiveStyle::with_stroke(Rgb565::WHITE, 1))
                    .draw(&mut lcd)
                    .unwrap();
            },
        };
    }
}
