use crate::{TouchEvent, TouchEventType, TouchScreen};
use embedded_hal::spi::SpiDevice;

mod spi;

pub struct Xpt2046<SPI, CALIB>
where 
    SPI: SpiDevice,
    CALIB: Fn((u16, u16)) -> Option<(i32, i32)>,
{
    spi: spi::Spi<SPI>,
    last_touch: Option<(i32, i32)>,
    calibration: CALIB,
}

impl<SPI, CALIB> Xpt2046<SPI, CALIB>
where 
    SPI: SpiDevice,
    CALIB: Fn((u16, u16)) -> Option<(i32, i32)>,
{
    pub fn new(touch_spi_device: SPI, calibration: CALIB) -> Self {
        Self {
            spi: spi::Spi::new(touch_spi_device),
            last_touch: None,
            calibration,
        }
    }
}

impl<SPI, CALIB> TouchScreen for Xpt2046<SPI, CALIB>
where 
    SPI: SpiDevice,
    CALIB: Fn((u16, u16)) -> Option<(i32, i32)>,
{
    type TouchError = <SPI as embedded_hal::spi::ErrorType>::Error;

    fn get_touch_event(&mut self) -> Result<Option<TouchEvent>, Self::TouchError> {
        match (self.calibration)(self.spi.get()?) {
            Some((x, y)) => {
                let result = Some(TouchEvent {
                    x,
                    y,
                    r#type: if self.last_touch.is_some() {
                        TouchEventType::Move
                    } else {
                        TouchEventType::Start
                    },
                });
                self.last_touch = Some((x, y));

                Ok(result)
            }
            None => {
                if let Some((last_x, last_y)) = self.last_touch {
                    self.last_touch = None;

                    Ok(Some(TouchEvent {
                        x: last_x,
                        y: last_y,
                        r#type: TouchEventType::End,
                    }))
                } else {
                    Ok(None)
                }
            }
        }
    }
}

pub fn default_calibration((x, y): (u16, u16)) -> Option<(i32, i32)> {
    if x < 250 || y < 230 || x > 4000 || y > 3900 {
        return None;
    }

    // rough but fast
    Some((
        ((x - 250).wrapping_shr(6) * 9).into(),
        ((y - 230).wrapping_shr(6) * 6).into(),
    ))
}

#[cfg(test)]
mod test {
    extern crate std;

    #[test]
    fn test_convert() {
        assert_eq!(super::default_calibration((250, 230)), Some((0, 0)));
        assert_eq!(super::default_calibration((3920, 3850)), Some((513, 336)));
    }

    #[test]
    fn test_convert_out_of_range() {
        assert_eq!(super::default_calibration((200, 200)), None);
        assert_eq!(super::default_calibration((4000, 4000)), None);
    }
}
