pub struct Builder {
    channel_select: ChannelSelect,
    bit_depth: BitDepth,
    diff_mode: DiffMode,
    power_down: PowerDown,
}
impl Builder {
    pub fn new() -> Self {
        Self {
            channel_select: ChannelSelect::default(),
            bit_depth: BitDepth::default(),
            diff_mode: DiffMode::default(),
            power_down: PowerDown::default(),
        }
    }
    pub fn channel_select(mut self, channel_select: ChannelSelect) -> Self {
        self.channel_select = channel_select;
        self
    }
    pub fn bit_depth(mut self, bit_depth: BitDepth) -> Self {
        self.bit_depth = bit_depth;
        self
    }
    pub fn diff_mode(mut self, diff_mode: DiffMode) -> Self {
        self.diff_mode= diff_mode;
        self
    }
    pub fn power_down(mut self, power_down: PowerDown) -> Self {
        self.power_down = power_down;
        self
    }
    pub fn build(self) -> u8 {
        let mut control_byte = 0x80u8;
        control_byte |= (self.channel_select as u8) << 4;
        control_byte |= (self.bit_depth as u8) << 3;
        control_byte |= (self.diff_mode as u8) << 2;
        control_byte |= (self.power_down as u8) << 0;
        control_byte
    }
}

#[derive(Clone, Copy, Debug, Default)]
#[allow(unused)]
pub(crate) enum ChannelSelect {
    #[default]
    Temperature0 = 0,
    XPosition = 1,
    Battery = 2,
    Z1 = 3,
    Z2 = 4,
    YPosition = 5,
    Auxiliary = 6,
    Temperature7 = 7,
}
impl TryFrom<u8> for ChannelSelect {
    type Error = ();
    fn try_from(value: u8) -> Result<Self, Self::Error> {
        match value {
            v if v == ChannelSelect::XPosition as u8 => Ok(Self::XPosition),
            v if v == ChannelSelect::YPosition as u8 => Ok(Self::YPosition),
            _ => Err(()),
        }
    }
}
impl From<ChannelSelect> for u8 {
    fn from(value: ChannelSelect) -> Self {
        value as Self
    }
}

#[derive(Clone, Copy, Debug, Default)]
pub(crate) enum BitDepth {
    #[default]
    Twelve = 0,
    Eight = 1,
}
impl TryFrom<u8> for BitDepth {
    type Error = ();
    fn try_from(value: u8) -> Result<Self, Self::Error> {
        match value {
            v if v == Self::Twelve as u8 => Ok(Self::Twelve),
            v if v == Self::Eight as u8 => Ok(Self::Eight),
            _ => Err(()),
        }
    }
}
impl From<BitDepth> for u8 {
    fn from(value: BitDepth) -> Self {
        value as Self
    }
}

#[derive(Clone, Copy, Debug, Default)]
pub(crate) enum DiffMode {
    #[default]
    Differential = 0,
    SingleEnded = 1,
}
impl TryFrom<u8> for DiffMode {
    type Error = ();
    fn try_from(value: u8) -> Result<Self, Self::Error> {
        match value {
            v if v == Self::Differential as u8 => Ok(Self::Differential),
            v if v == Self::SingleEnded as u8 => Ok(Self::SingleEnded),
            _ => Err(()),
        }
    }
}
impl From<DiffMode> for u8 {
    fn from(value: DiffMode) -> Self {
        value as Self
    }
}

#[derive(Clone, Copy, Debug, Default)]
pub(crate) enum PowerDown {
    #[default]
    OffBetweenConversions = 0,
    ReferenceOffAdcOn = 1,
    ReferenceOnAdcOff = 2,
    AlwaysPowered = 3,
}
impl TryFrom<u8> for PowerDown {
    type Error = ();
    fn try_from(value: u8) -> Result<Self, Self::Error> {
        match value {
            v if v == Self::OffBetweenConversions as u8 => Ok(Self::OffBetweenConversions),
            v if v == Self::ReferenceOffAdcOn as u8 => Ok(Self::ReferenceOffAdcOn),
            v if v == Self::ReferenceOnAdcOff as u8 => Ok(Self::ReferenceOnAdcOff),
            v if v == Self::AlwaysPowered as u8 => Ok(Self::AlwaysPowered),
            _ => Err(()),
        }
    }
}
impl From<PowerDown> for u8 {
    fn from(value: PowerDown) -> Self {
        value as Self
    }
}

#[cfg(test)]
mod test {
    extern crate std;

    #[test]
    fn test_start_bit_default() {
        let control_byte = super::Builder::new()
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 7) & 1u8, 1);
    }

    #[test]
    fn test_start_bit_contamination_ones() {
        let control_byte = super::Builder::new()
            .channel_select(super::ChannelSelect::Temperature7)
            .bit_depth(super::BitDepth::Eight)
            .diff_mode(super::DiffMode::SingleEnded)
            .power_down(super::PowerDown::AlwaysPowered)
            .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 7) & 1u8, 1);
    }

    #[test]
    fn test_start_bit_contamination_zeros() {
        let control_byte = super::Builder::new()
            .channel_select(super::ChannelSelect::Temperature0)
            .bit_depth(super::BitDepth::Twelve)
            .diff_mode(super::DiffMode::Differential)
            .power_down(super::PowerDown::OffBetweenConversions)
            .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 7) & 1u8, 1);
    }

    #[test]
    fn test_channel_select_default() {
        let control_byte = super::Builder::new()
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 4) & 7u8, super::ChannelSelect::default() as u8);
    }

    #[test]
    fn test_channel_select_contamination() {
        let control_byte = super::Builder::new()
            .bit_depth(super::BitDepth::Eight)
            .diff_mode(super::DiffMode::SingleEnded)
            .power_down(super::PowerDown::AlwaysPowered)
            .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 4) & 7u8, super::ChannelSelect::default() as u8);
    }

    #[test]
    fn test_channel_select_temperature0() {
        let control_byte = super::Builder::new()
                .channel_select(super::ChannelSelect::Temperature0)
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 4) & 7u8, super::ChannelSelect::Temperature0 as u8);
    }

    #[test]
    fn test_channel_select_x_position() {
        let control_byte = super::Builder::new()
                .channel_select(super::ChannelSelect::XPosition)
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 4) & 7u8, super::ChannelSelect::XPosition as u8);
    }

    #[test]
    fn test_channel_select_battery() {
        let control_byte = super::Builder::new()
                .channel_select(super::ChannelSelect::Battery)
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 4) & 7u8, super::ChannelSelect::Battery as u8);
    }

    #[test]
    fn test_channel_select_z1() {
        let control_byte = super::Builder::new()
                .channel_select(super::ChannelSelect::Z1)
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 4) & 7u8, super::ChannelSelect::Z1 as u8);
    }

    #[test]
    fn test_channel_select_z2() {
        let control_byte = super::Builder::new()
                .channel_select(super::ChannelSelect::Z2)
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 4) & 7u8, super::ChannelSelect::Z2 as u8);
    }

    #[test]
    fn test_channel_select_y_position() {
        let control_byte = super::Builder::new()
                .channel_select(super::ChannelSelect::YPosition)
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 4) & 7u8, super::ChannelSelect::YPosition as u8);
    }

    #[test]
    fn test_channel_select_auxiliary() {
        let control_byte = super::Builder::new()
                .channel_select(super::ChannelSelect::Auxiliary)
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 4) & 7u8, super::ChannelSelect::Auxiliary as u8);
    }

    #[test]
    fn test_channel_select_temperature7() {
        let control_byte = super::Builder::new()
                .channel_select(super::ChannelSelect::Temperature7)
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 4) & 7u8, super::ChannelSelect::Temperature7 as u8);
    }

    #[test]
    fn test_bit_depth_default() {
        let control_byte = super::Builder::new()
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 3) & 1u8, super::BitDepth::default() as u8);
    }

    #[test]
    fn test_bit_depth_contamination() {
        let control_byte = super::Builder::new()
                .channel_select(super::ChannelSelect::Temperature7)
                .diff_mode(super::DiffMode::SingleEnded)
                .power_down(super::PowerDown::AlwaysPowered)
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 3) & 1u8, super::BitDepth::default() as u8);
    }

    #[test]
    fn test_bit_depth_twelve() {
        let control_byte = super::Builder::new()
                .bit_depth(super::BitDepth::Twelve)
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 3) & 1u8, super::BitDepth::Twelve as u8);
    }

    #[test]
    fn test_bit_depth_eight() {
        let control_byte = super::Builder::new()
                .bit_depth(super::BitDepth::Eight)
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 3) & 1u8, super::BitDepth::Eight as u8);
    }

    #[test]
    fn test_diff_mode_default() {
        let control_byte = super::Builder::new()
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 2) & 1u8, super::DiffMode::default() as u8);
    }

    #[test]
    fn test_diff_mode_contamination() {
        let control_byte = super::Builder::new()
                .channel_select(super::ChannelSelect::Temperature7)
                .bit_depth(super::BitDepth::Eight)
                .power_down(super::PowerDown::AlwaysPowered)
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 2) & 1u8, super::DiffMode::default() as u8);
    }

    #[test]
    fn test_diff_mode_differential() {
        let control_byte = super::Builder::new()
                .diff_mode(super::DiffMode::Differential)
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 2) & 1u8, super::DiffMode::Differential as u8);
    }

    #[test]
    fn test_diff_mode_single_ended() {
        let control_byte = super::Builder::new()
                .diff_mode(super::DiffMode::SingleEnded)
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 2) & 1u8, super::DiffMode::SingleEnded as u8);
    }

    #[test]
    fn test_power_down_default() {
        let control_byte = super::Builder::new()
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 0) & 3u8, super::PowerDown::default() as u8);
    }

    #[test]
    fn test_power_down_contamination() {
        let control_byte = super::Builder::new()
                .channel_select(super::ChannelSelect::Temperature7)
                .bit_depth(super::BitDepth::Eight)
                .diff_mode(super::DiffMode::SingleEnded)
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 0) & 3u8, super::PowerDown::default() as u8);
    }

    #[test]
    fn test_power_down_off_between_conversions() {
        let control_byte = super::Builder::new()
                .power_down(super::PowerDown::OffBetweenConversions)
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 0) & 3u8, super::PowerDown::OffBetweenConversions as u8);
    }

    #[test]
    fn test_power_down_reference_off_adc_on() {
        let control_byte = super::Builder::new()
                .power_down(super::PowerDown::ReferenceOffAdcOn)
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 0) & 3u8, super::PowerDown::ReferenceOffAdcOn as u8);
    }

    #[test]
    fn test_power_down_reference_on_adc_off() {
        let control_byte = super::Builder::new()
                .power_down(super::PowerDown::ReferenceOnAdcOff)
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 0) & 3u8, super::PowerDown::ReferenceOnAdcOff as u8);
    }

    #[test]
    fn test_power_down_always_powered() {
        let control_byte = super::Builder::new()
                .power_down(super::PowerDown::AlwaysPowered)
                .build();
        std::println!("control_byte=0x{control_byte:02x}");
        assert_eq!((control_byte >> 0) & 3u8, super::PowerDown::AlwaysPowered as u8);
    }
}
