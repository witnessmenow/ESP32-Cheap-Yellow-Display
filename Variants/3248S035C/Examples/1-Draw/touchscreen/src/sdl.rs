use crate::{TouchEvent, TouchEventType, TouchScreen};
use core::convert::From;
use embedded_graphics_core::{
    pixelcolor::{PixelColor, Rgb888},
    prelude::{DrawTarget, OriginDimensions, Size},
    Pixel,
};
use embedded_graphics_simulator::{SimulatorDisplay, SimulatorEvent, Window};

pub struct Sdl<C>
where
    C: PixelColor,
{
    simulator_display: SimulatorDisplay<C>,
    window: Window,
}

impl<C> Sdl<C>
where
    C: PixelColor,
{
    #[must_use]
    pub fn new(simulator_display: SimulatorDisplay<C>, window: Window) -> Self {
        Self {
            simulator_display,
            window,
        }
    }
}

impl<C> DrawTarget for Sdl<C>
where
    C: PixelColor + Into<Rgb888> + From<Rgb888>,
{
    type Color = C;
    type Error = core::convert::Infallible;

    fn draw_iter<I: IntoIterator<Item = Pixel<<Self as DrawTarget>::Color>>>(
        &mut self,
        i: I,
    ) -> Result<(), <Self as DrawTarget>::Error> {
        self.simulator_display.draw_iter(i)
    }
}

impl<C> OriginDimensions for Sdl<C>
where
    C: PixelColor,
{
    fn size(&self) -> Size {
        self.simulator_display.size()
    }
}

impl<C> TouchScreen for Sdl<C>
where
    C: PixelColor + From<Rgb888>,
    Rgb888: From<C>,
{
    type TouchError = core::convert::Infallible;

    fn get_touch_event(&mut self) -> Result<Option<TouchEvent>, Self::TouchError> {
        self.window.update(&self.simulator_display);

        if let Some(event) = self.window.events().last() {
            match event {
                SimulatorEvent::Quit => panic!(),
                SimulatorEvent::MouseButtonUp { point, .. } => Ok(Some(TouchEvent {
                    r#type: TouchEventType::Start,
                    x: point.x,
                    y: point.y,
                })),
                SimulatorEvent::MouseButtonDown { point, .. } => Ok(Some(TouchEvent {
                    r#type: TouchEventType::End,
                    x: point.x,
                    y: point.y,
                })),
                SimulatorEvent::MouseMove { point, .. } => Ok(Some(TouchEvent {
                    r#type: TouchEventType::Move,
                    x: point.x,
                    y: point.y,
                })),
                _ => Ok(None),
            }
        } else {
            Ok(None)
        }
    }
}
