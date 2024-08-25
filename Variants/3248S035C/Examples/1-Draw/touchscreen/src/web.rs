extern crate std;
use crate::{TouchEvent, TouchEventType, TouchScreen};
use embedded_graphics_core::{
    pixelcolor::{PixelColor, Rgb888},
    prelude::{DrawTarget, OriginDimensions, Size},
    Pixel,
};

use embedded_graphics_web_simulator::display::WebSimulatorDisplay;
pub use embedded_graphics_web_simulator::output_settings::{OutputSettings, OutputSettingsBuilder};
use std::boxed::Box;
use std::{
    error::Error,
    sync::mpsc::{channel, Receiver, Sender},
};
use wasm_bindgen::prelude::*;
use web_sys::{Element, HtmlElement};

enum MouseEventType {
    Down,
    Move,
    Up,
    Leave,
}

struct MouseEvent {
    x: i32,
    y: i32,
    r#type: MouseEventType,
}

pub struct Web<C>
where
    C: PixelColor + Into<Rgb888>,
{
    simulator_display: WebSimulatorDisplay<C>,
    down: bool,
    channel: (Sender<MouseEvent>, Receiver<MouseEvent>),
}

impl<C> Web<C>
where
    C: PixelColor + Into<Rgb888>,
{
    #[must_use]
    pub fn new(size: (u32, u32), output_settings: &OutputSettings, element: &Element) -> Self {
        let simulator_display = WebSimulatorDisplay::new(size, output_settings, Some(element));

        let html_element = element.dyn_ref::<HtmlElement>().unwrap();

        let result = Self {
            simulator_display,
            down: false,
            channel: channel(),
        };

        {
            let sender = result.channel.0.clone();
            let mousedown_closure =
                Closure::<dyn FnMut(_)>::new(move |event: web_sys::MouseEvent| {
                    sender
                        .send(MouseEvent {
                            x: event.offset_x(),
                            y: event.offset_y(),
                            r#type: MouseEventType::Down,
                        })
                        .unwrap();
                });

            html_element
                .add_event_listener_with_callback(
                    "mousedown",
                    mousedown_closure.as_ref().unchecked_ref(),
                )
                .unwrap();

            mousedown_closure.forget();
        }

        {
            let sender = result.channel.0.clone();
            let mousemove_closure =
                Closure::<dyn FnMut(_)>::new(move |event: web_sys::MouseEvent| {
                    sender
                        .send(MouseEvent {
                            x: event.offset_x(),
                            y: event.offset_y(),
                            r#type: MouseEventType::Move,
                        })
                        .unwrap();
                });

            html_element
                .add_event_listener_with_callback(
                    "mousemove",
                    mousemove_closure.as_ref().unchecked_ref(),
                )
                .unwrap();

            mousemove_closure.forget();
        }

        {
            let sender = result.channel.0.clone();
            let mouseup_closure =
                Closure::<dyn FnMut(_)>::new(move |event: web_sys::MouseEvent| {
                    sender
                        .send(MouseEvent {
                            x: event.offset_x(),
                            y: event.offset_y(),
                            r#type: MouseEventType::Up,
                        })
                        .unwrap();
                });

            html_element
                .add_event_listener_with_callback(
                    "mouseup",
                    mouseup_closure.as_ref().unchecked_ref(),
                )
                .unwrap();

            mouseup_closure.forget();
        }

        {
            let sender = result.channel.0.clone();
            let mouseleave_closure =
                Closure::<dyn FnMut(_)>::new(move |event: web_sys::MouseEvent| {
                    sender
                        .send(MouseEvent {
                            x: event.offset_x(),
                            y: event.offset_y(),
                            r#type: MouseEventType::Leave,
                        })
                        .unwrap();
                });

            html_element
                .add_event_listener_with_callback(
                    "mouseleave",
                    mouseleave_closure.as_ref().unchecked_ref(),
                )
                .unwrap();

            mouseleave_closure.forget();
        }

        result
    }
}

impl<C> DrawTarget for Web<C>
where
    C: PixelColor + Into<Rgb888>,
{
    type Color = C;
    type Error = Box<dyn Error>;

    fn draw_iter<I: IntoIterator<Item = Pixel<<Self as DrawTarget>::Color>>>(
        &mut self,
        i: I,
    ) -> Result<(), <Self as DrawTarget>::Error> {
        let result = self.simulator_display.draw_iter(i);
        self.simulator_display.flush().unwrap();
        result
    }
}

impl<C> OriginDimensions for Web<C>
where
    C: PixelColor + Into<Rgb888>,
{
    fn size(&self) -> Size {
        self.simulator_display.size()
    }
}

impl<C> TouchScreen for Web<C>
where
    C: PixelColor + Into<Rgb888>,
{
    // not really infallible, the mouse handlers can fail but these failures are not being
    // collected right now
    type TouchError = std::convert::Infallible;

    fn get_touch_event(&mut self) -> Result<Option<TouchEvent>, Self::TouchError> {
        match self.channel.1.try_recv() {
            Ok(MouseEvent {
                x,
                y,
                r#type: MouseEventType::Down,
            }) => {
                self.down = true;
                Ok(Some(TouchEvent {
                    x,
                    y,
                    r#type: TouchEventType::Start,
                }))
            }
            Ok(MouseEvent {
                x,
                y,
                r#type: MouseEventType::Up | MouseEventType::Leave,
            }) => {
                if self.down {
                    self.down = false;
                    Ok(Some(TouchEvent {
                        x,
                        y,
                        r#type: TouchEventType::End,
                    }))
                } else {
                    Ok(None)
                }
            }
            Ok(MouseEvent {
                x,
                y,
                r#type: MouseEventType::Move,
            }) => {
                if self.down {
                    Ok(Some(TouchEvent {
                        x,
                        y,
                        r#type: TouchEventType::Move,
                    }))
                } else {
                    Ok(None)
                }
            }
            _ => Ok(None),
        }
    }
}
