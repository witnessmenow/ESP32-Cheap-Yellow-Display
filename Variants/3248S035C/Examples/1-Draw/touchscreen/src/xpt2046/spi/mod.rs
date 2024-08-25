mod control_byte;
use control_byte::{
    Builder as ControlByteBuilder,
    ChannelSelect,
    BitDepth,
    DiffMode,
    PowerDown,
};

use embedded_hal::spi::SpiDevice;

pub struct Spi<SPI: SpiDevice>(SPI);

impl<SPI: SpiDevice> Spi<SPI> {
    pub fn new(spi_device: SPI) -> Self {
        Self(spi_device)
    }

    pub fn get(&mut self) -> Result<(u16, u16), <SPI as embedded_hal::spi::ErrorType>::Error> {
        let mut buf = [0u8; 5];

        let control_byte = ControlByteBuilder::new()
            .channel_select(ChannelSelect::XPosition)
            .bit_depth(BitDepth::Twelve)
            .diff_mode(DiffMode::Differential)
            .power_down(PowerDown::OffBetweenConversions)
            .build();
        // buf[0] = control_byte;
        buf[0] = control_byte >> 3;
        buf[1] = control_byte << 5;

        let control_byte = ControlByteBuilder::new()
            .channel_select(ChannelSelect::YPosition)
            .bit_depth(BitDepth::Twelve)
            .diff_mode(DiffMode::Differential)
            .power_down(PowerDown::OffBetweenConversions)
            .build();
        // buf[2] = control_byte;
        buf[3] = control_byte << 5;
        buf[2] = control_byte >> 3;

        cfg_if::cfg_if! {
            if #[cfg(feature = "log")] {
                log::info!("tx_buf={buf:?}");
            }
        }
        self.0.transfer_in_place(&mut buf)?;
        cfg_if::cfg_if! {
            if #[cfg(feature = "log")] {
                log::info!("rx_buf={buf:?}");
            }
        }

        Ok((
            u16::from_be_bytes([buf[1], buf[2]]),
            u16::from_be_bytes([buf[3], buf[4]]),
        ))
    }
}

#[cfg(test)]
mod test {
    use embedded_hal_mock::eh1::spi::{Mock as SpiMock, Transaction as SpiTransaction};
    extern crate std;

    #[test]
    fn test_get() {
        let expectations = [
            SpiTransaction::transaction_start(),
            SpiTransaction::transfer_in_place(
                std::vec![0x12, 0x00, 0x1A, 0x00, 0x00],
                std::vec![0x00, 0x00, 0x00, 0x00, 0x00],
            ),
            SpiTransaction::transaction_end(),

            SpiTransaction::transaction_start(),
            SpiTransaction::transfer_in_place(
                std::vec![0x12, 0x00, 0x1A, 0x00, 0x00],
                std::vec![0x00, 0x10, 0x00, 0x10, 0x00],
            ),
            SpiTransaction::transaction_end(),

            SpiTransaction::transaction_start(),
            SpiTransaction::transfer_in_place(
                std::vec![0x12, 0x00, 0x1A, 0x00, 0x00],
                std::vec![0x00, 0x00, 0x00, 0x10, 0x00],
            ),
            SpiTransaction::transaction_end(),

            SpiTransaction::transaction_start(),
            SpiTransaction::transfer_in_place(
                std::vec![0x12, 0x00, 0x1A, 0x00, 0x00],
                std::vec![0x00, 0x10, 0x00, 0x00, 0x00],
            ),
            SpiTransaction::transaction_end(),

            SpiTransaction::transaction_start(),
            SpiTransaction::transfer_in_place(
                std::vec![0x12, 0x00, 0x1A, 0x00, 0x00],
                std::vec![0x00, 0x07, 0xD0, 0x00, 0x00],
            ),
            SpiTransaction::transaction_end(),

            SpiTransaction::transaction_start(),
            SpiTransaction::transfer_in_place(
                std::vec![0x12, 0x00, 0x1A, 0x00, 0x00],
                std::vec![0x00, 0x00, 0x00, 0x07, 0xD0],
            ),
            SpiTransaction::transaction_end(),
        ];

        let mut spi = SpiMock::new(&expectations);


        std::println!("round 0");
        let actual = super::Spi::new(spi.clone()).get();
        std::println!("actual={actual:?}");
        assert_eq!(actual, Ok((0, 0)));

        std::println!("round 1");
        let actual = super::Spi::new(spi.clone()).get();
        std::println!("actual={actual:?}");
        assert_eq!(actual, Ok((4096, 4096)));

        std::println!("round 2");
        let actual = super::Spi::new(spi.clone()).get();
        std::println!("actual={actual:?}");
        assert_eq!(actual, Ok((0, 4096)));

        std::println!("round 3");
        let actual = super::Spi::new(spi.clone()).get();
        std::println!("actual={actual:?}");
        assert_eq!(actual, Ok((4096, 0)));


        std::println!("round 4");
        let actual = super::Spi::new(spi.clone()).get();
        std::println!("actual={actual:?}");
        assert_eq!(actual, Ok((2000, 0)));

        std::println!("round 5");
        let actual = super::Spi::new(spi.clone()).get();
        std::println!("actual={actual:?}");
        assert_eq!(actual, Ok((0, 2000)));

        spi.done();
    }
}
