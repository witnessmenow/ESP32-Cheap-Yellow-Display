using System;
using System.Diagnostics;
using System.Drawing;
using System.Threading;
using nanoFramework.Hardware.Esp32;
using nanoFramework.UI;
using nanoFramework.UI.GraphicDrivers;
using System.Device.Gpio;

namespace ScreenTest
{
    public class Program
    {
        private const int ChipSelect = 15;
        private const int DataCommand = 2;
        private const int Reset = 4;
        private const int BackLight = 21;

        private const int Width = 320;
        private const int Height = 240;

        private static GpioController _sGpioController;

        public static void Main()
        {
            _sGpioController = new GpioController();

            // If you're using an ESP32, use nanoFramework.Hardware.Esp32 to remap the SPI pins
            Configuration.SetPinFunction(13, DeviceFunction.SPI1_MOSI);
            Configuration.SetPinFunction(14, DeviceFunction.SPI1_CLOCK);
            // This is not used but must be defined
            Configuration.SetPinFunction(12, DeviceFunction.SPI1_MISO);

            Debug.WriteLine("Hello from nanoFramework!");

            Debug.WriteLine("Pins Successfully configured");

            var displaySpiConfig = new SpiConfiguration(
                1,
                ChipSelect,
                DataCommand,
                Reset,
                BackLight);

            var graphicDriver = Ili9341.GraphicDriverWithDefaultManufacturingSettings;

            var screenConfig = new ScreenConfiguration(
                0,
                0,
                Width,
                Height,
                graphicDriver);

            var init = DisplayControl.Initialize(
                displaySpiConfig,
                screenConfig );

            // Depending on you ESP32, you may also have to use either PWM either GPIO to set the backlight pin mode on
            _sGpioController.OpenPin(BackLight, PinMode.Output);
            _sGpioController.Write(BackLight, PinValue.High);


            DisplayControl.Clear();

            Debug.WriteLine($"init screen initialized");

            ushort[] toSend = new ushort[50*Width];
            var blue = Color.Blue.ToBgr565();
            var red = Color.Red.ToBgr565();
            var green = Color.Green.ToBgr565();
            var white = Color.White.ToBgr565();

            for (int i = 0; i < toSend.Length; i++)
            {
                toSend[i] = blue;
            }

            DisplayControl.Write(0, 0, 50, 50, toSend);

            for (int i = 0; i < toSend.Length; i++)
            {
                toSend[i] = green;
            }

            DisplayControl.Write(0, 51, 50, 50, toSend);

            for (int i = 0; i < toSend.Length; i++)
            {
                toSend[i] = white;
            }

            DisplayControl.Write(0, 101, 50, 50, toSend);

            Thread.Sleep(Timeout.Infinite);
        }
    }
}
