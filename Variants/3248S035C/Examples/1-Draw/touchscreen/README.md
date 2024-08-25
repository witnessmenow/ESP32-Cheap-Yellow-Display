# Touchscreen

Important NOTE: This is a modified version of that published on crates.io.

[![touchscreen on crates.io](https://img.shields.io/crates/d/touchscreen)](https://crates.io/crates/touchscreen)
[![touchscreen on docs.rs](https://img.shields.io/docsrs/touchscreen)](https://docs.rs/touchscreen/latest/touchscreen/)

A touchscreen is just a screen that you can touch.  Central to this crate is
the Touchscreen trait:

```rust
pub trait Touchscreen:
    embedded_graphics_core::prelude::DrawTarget +
    embedded_graphics_core::prelude::OriginDimensions {
        type TouchError;

        fn get_touch_event(&mut self) -> Result<Option<TouchEvent>, Self::TouchError>;
    }
```

It is overly simple right now, it is more of a tentative starting point.  There
are 2 implementations, which can be selected by feature flags:

- `xpt2046-screen`: A screen that combines XPT2046 (touch) with arbitrary embedded-graphics `DrawTarget`.
- `web-screen`: A screen that marries WebSimulatorDisplay with mouse event handlers bound to a container element.
- `sdl-screen`: SimulatorDisplay adapted to Touchscreen.  Unfinished

## Todo

- Support touchscreens that can signal touch with interrupt
- Finish SDL `touchscreen`
- Clean up `web_screen`, propagate mouse handling errors
