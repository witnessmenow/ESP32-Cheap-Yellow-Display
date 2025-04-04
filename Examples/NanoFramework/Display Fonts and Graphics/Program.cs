using nanoFramework.Hardware.Esp32;
using nanoFramework.UI;
using System;
using System.Diagnostics;
using System.Threading;
using System.Drawing;
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

        const int textPosY = 10;
        const string text1 = @"Congratulations";
        const string text2 = @"You Have";
        const string text3 = @"nanoFramework";
        const string text4 = @"On your CYD";


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
            //Needed to customise the ILI9341 driver to work with the board as the X axis was mirrored.
            graphicDriver.DefaultOrientation = DisplayOrientation.Landscape;
            
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
            
            BlankScreen(Color.White);

            Debug.WriteLine($"init screen initialized");
            
            DisplayText();

            //Logo
            BlitBitmap(BitmapFile.Logo,5,195, 40,40);

            Thread.Sleep(Timeout.Infinite);
        }

        

        private static void DisplayText()
        {
            Font DisplayFont = Resource.GetFont(Resource.FontResources.CenturyGothicBold36);

            var fontColor = ColorToABRGColor(Color.DarkBlue);
            
            DisplayControl.Write(text1, xPosCentredText(DisplayFont,text1,Width), textPosY, Width, Height, DisplayFont, fontColor, Color.White);
            DisplayControl.Write(text2, xPosCentredText(DisplayFont, text2, Width), (ushort)(textPosY + DisplayFont.Height), Width, Height, DisplayFont, fontColor, Color.White);
            DisplayControl.Write(text3, xPosCentredText(DisplayFont, text3, Width), (ushort)(textPosY + 2* DisplayFont.Height), Width, Height, DisplayFont, fontColor, Color.White);
            DisplayControl.Write(text4, xPosCentredText(DisplayFont, text4, Width), (ushort)(textPosY + 3* DisplayFont.Height), Width, Height, DisplayFont, fontColor, Color.White);
        }

        private static void BlitBitmap(ushort[] bitmap, ushort xStart, ushort yStart, ushort bWidth, ushort bHeight)
        {
            ushort[] buffer = new ushort[1];
            int index = 0;

            for (ushort x = 0; x < bWidth; x++)
            {
                for (ushort y = 0; y < bHeight; y++)
                {
                    buffer[0] = bitmap[index];
                    DisplayControl.Write((ushort)(x+ xStart), (ushort)(y + yStart), 1, 1, buffer);
                    index++;
                }
            }
        }

        private static void BlankScreen(Color screenColor)
        {
            DisplayControl.Clear();

            ushort[] toSend = new ushort[100];
            var colourBgr = screenColor.ToBgr565();
            for (int i = 0; i < toSend.Length; i++)
            {
                toSend[i] = colourBgr;
            }

            for (ushort x = 0; x < Width; x += 10)
            {
                for (ushort y = 0; y < Height; y += 10)
                {
                    DisplayControl.Write(x, y, 10, 10, toSend);
                }
            }
        }

        private static ushort xPosCentredText(Font displayFont, string text, int screenWidth)
        {
            var fontWidth = displayFont.AverageWidth + 1;
            var textWidth = (text.Length * fontWidth);
            var textPos = (screenWidth - textWidth) / 2;
            return (ushort)(textPos < 0 ? 0 : textPos);
        }

        //Board is BRG not RGB so we need to remap any colours.
        private static Color ColorToABRGColor(Color rgbColor )
        {
            return Color.FromHex($"#{rgbColor.ToBgr565():X}");
        }
    }
}

