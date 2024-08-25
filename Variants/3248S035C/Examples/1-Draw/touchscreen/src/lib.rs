#![no_std]

#[derive(Debug, PartialEq, Eq)]
pub enum TouchEventType {
    Start,
    Move,
    End,
}

#[derive(Debug, PartialEq, Eq)]
pub struct TouchEvent {
    pub x: i32,
    pub y: i32,
    pub r#type: TouchEventType,
}

pub trait TouchScreen {
    type TouchError;

    fn get_touch_event(&mut self) -> Result<Option<TouchEvent>, Self::TouchError>;
}

#[cfg(feature = "xpt2046")]
pub mod xpt2046;

#[cfg(feature = "web")]
pub mod web;

#[cfg(feature = "sdl")]
pub mod sdl;
