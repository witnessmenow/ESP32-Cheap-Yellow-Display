using System;
using System.Threading;
using System.Diagnostics;
using System.Device.Gpio;

namespace Blinky 
{
    public class Program
    {
        private static GpioController s_GpioController;
        public static void Main()
        {
            s_GpioController = new GpioController();

            GpioPin ledR = s_GpioController.OpenPin(4, PinMode.Output);
            GpioPin ledG = s_GpioController.OpenPin(16, PinMode.Output);
            GpioPin ledB = s_GpioController.OpenPin(17, PinMode.Output);

            Debug.WriteLine("LED Demo");

            while (true)
            {
                //LEDs are active low so set them high to turn them off
                ledR.Write(PinValue.High);
                ledG.Write(PinValue.High);
                ledB.Write(PinValue.High);

                Thread.Sleep(1000);

                //Red LED
                ledR.Write(PinValue.Low);
                ledG.Write(PinValue.High);
                ledB.Write(PinValue.High);

                Thread.Sleep(1000);

                //Green LED
                ledR.Write(PinValue.High);
                ledG.Write(PinValue.Low);
                ledB.Write(PinValue.High);

                Thread.Sleep(1000);

                //Blue LED
                ledR.Write(PinValue.High);
                ledG.Write(PinValue.High);
                ledB.Write(PinValue.Low);

                Thread.Sleep(1000);
            }
        }

        static int PinNumber(char port, byte pin)
        {
            if (port < 'A' || port > 'J')
                throw new ArgumentException();

            return ((port - 'A') * 16) + pin;
        }
    }
}
