//
// Copyright (c) .NET Foundation and Contributors
// Portions Copyright (c) Microsoft Corporation.  All rights reserved.
// See LICENSE file in the project root for full license information.
//

using System;

namespace nanoFramework.UI.GraphicDrivers
{
    /// <summary>
    /// Managed driver for Ili9341.
    /// </summary>
    public static class Ili9341
    {
        private static GraphicDriver _driver;
        private static GraphicDriver _driverDefaultSettings;

        // Those enums are left like this to match the native side
        private enum ILI9341_CMD
        {
            NOP = 0x00,
            SOFTWARE_RESET = 0x01,
            Read_Display_Identification_Information = 0x04,
            POWER_STATE = 0x10,
            Sleep_Out = 0x11,
            Normal_Display_On = 0x13,
            Noron = 0x13,
            Invert_Off = 0x20,
            Invert_On = 0x21,
            Gamma_Set = 0x26,
            Display_OFF = 0x28,
            Display_ON = 0x29,
            Column_Address_Set = 0x2A,
            Page_Address_Set = 0x2B,
            Memory_Write = 0x2C,
            Colour_Set = 0x2D,
            Memory_Read = 0x2E,
            Partial_Area = 0x30,
            Memory_Access_Control = 0x36,
            Pixel_Format_Set = 0x3A,
            Memory_Write_Continue = 0x3C,
            Write_Display_Brightness = 0x51,
            Interface_Signal_Control = 0xB0,
            Frame_Rate_Control_Normal = 0xB1,
            Display_Function_Control = 0xB6,
            Entry_Mode_Set = 0xB7,
            Power_Control_1 = 0xC0,
            Power_Control_2 = 0xC1,
            VCOM_Control_1 = 0xC5,
            VCOM_Control_2 = 0xC7,
            External_Command = 0xC8,
            Power_Control_A = 0xCB,
            Power_Control_B = 0xCF,
            Positive_Gamma_Correction = 0xE0,
            Negative_Gamma_Correction = 0XE1,
            Driver_Timing_Control_A = 0xE8,
            Driver_Timing_Control_B = 0xEA,
            Power_On_Sequence = 0xED,
            Enable_3G = 0xF2,
            Interface_Control = 0xF6,
            Pump_Ratio_Control = 0xF7
        };

        [Flags]
        private enum ILI9341_Orientation
        {
            MADCTL_MH = 0x04, // sets the Horizontal Refresh, 0=Left-Right and 1=Right-Left
            MADCTL_ML = 0x10, // sets the Vertical Refresh, 0=Top-Bottom and 1=Bottom-Top
            MADCTL_MV = 0x20, // sets the Row/Column Swap, 0=Normal and 1=Swapped
            MADCTL_MX = 0x40, // sets the Column Order, 0=Left-Right and 1=Right-Left
            MADCTL_MY = 0x80, // sets the Row Order, 0=Top-Bottom and 1=Bottom-Top

            MADCTL_BGR = 0x08 // Blue-Green-Red pixel order
        };

        /// <summary>
        /// Default weight. Use to overrride the one you'll pass in the screen and add it to the driver.
        /// </summary>
        public static ushort Width { get; } = 240;

        /// <summary>
        /// Default height. Use to overrride the one you'll pass in the screen and add it to the driver.
        /// </summary>
        public static ushort Height { get; } = 320;

        /// <summary>
        /// Gets the graphic driver for the Ili9341 display.
        /// </summary>
        public static GraphicDriver GraphicDriver
        {
            get
            {
                if (_driver == null)
                {
                    _driver = new GraphicDriver()
                    {
                        BitsPerPixel = 16,
                        MemoryWrite = (byte)ILI9341_CMD.Memory_Write,
                        SetColumnAddress = (byte)ILI9341_CMD.Column_Address_Set,
                        SetRowAddress = (byte)ILI9341_CMD.Page_Address_Set,
                        InitializationSequence = new byte[]
                        {
                            (byte)GraphicDriverCommandType.Command, 4, (byte)ILI9341_CMD.Power_Control_B, 0x00, 0x83, 0X30,
                            (byte)GraphicDriverCommandType.Command, 5, (byte)ILI9341_CMD.Power_On_Sequence, 0x64, 0x03, 0X12, 0X81,
                            (byte)GraphicDriverCommandType.Command, 4, (byte)ILI9341_CMD.Driver_Timing_Control_A, 0x85, 0x01, 0x79,
                            (byte)GraphicDriverCommandType.Command, 6, (byte)ILI9341_CMD.Power_Control_A, 0x39, 0x2C, 0x00, 0x34, 0x02,
                            (byte)GraphicDriverCommandType.Command, 2, (byte)ILI9341_CMD.Pump_Ratio_Control, 0x20,
                            (byte)GraphicDriverCommandType.Command, 3, (byte)ILI9341_CMD.Driver_Timing_Control_B, 0x00, 0x00,
                            (byte)GraphicDriverCommandType.Command, 2, (byte)ILI9341_CMD.Power_Control_1, 0x26,
                            (byte)GraphicDriverCommandType.Command, 2, (byte)ILI9341_CMD.Power_Control_2, 0x11,
                            (byte)GraphicDriverCommandType.Command, 3, (byte)ILI9341_CMD.VCOM_Control_1, 0x35, 0x3E,
                            (byte)GraphicDriverCommandType.Command, 2, (byte)ILI9341_CMD.VCOM_Control_2, 0xBE,
                            (byte)GraphicDriverCommandType.Command, 2, (byte)ILI9341_CMD.Memory_Access_Control, 0x28,
                            (byte)GraphicDriverCommandType.Command, 2, (byte)ILI9341_CMD.Pixel_Format_Set, 0x55,
                            (byte)GraphicDriverCommandType.Command, 3, (byte)ILI9341_CMD.Frame_Rate_Control_Normal, 0x00, 0x1B,
                            (byte)GraphicDriverCommandType.Command, 2, (byte)ILI9341_CMD.Enable_3G, 0x08,
                            (byte)GraphicDriverCommandType.Command, 2, (byte)ILI9341_CMD.Gamma_Set, 0x01,
                            (byte)GraphicDriverCommandType.Command, 16, (byte)ILI9341_CMD.Positive_Gamma_Correction, 0x1F, 0x1A, 0x18, 0x0A, 0x0F, 0x06, 0x45, 0X87, 0x32, 0x0A, 0x07, 0x02, 0x07, 0x05, 0x00,
                            (byte)GraphicDriverCommandType.Command, 16, (byte)ILI9341_CMD.Negative_Gamma_Correction, 0x00, 0x25, 0x27, 0x05, 0x10, 0x09, 0x3A, 0x78, 0x4D, 0x05, 0x18, 0x0D, 0x38, 0x3A, 0x1F,
                            (byte)GraphicDriverCommandType.Command, 5, (byte)ILI9341_CMD.Column_Address_Set, 0, 0, (byte)((Height - 1) >> 8), (byte)((Height - 1) & 0xFF),
                            (byte)GraphicDriverCommandType.Command, 5, (byte)ILI9341_CMD.Page_Address_Set, 0, 0, (byte)((Width - 1) >> 8), (byte)((Width- 1) & 0xFF),
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.Memory_Write,
                            (byte)GraphicDriverCommandType.Command, 2, (byte)ILI9341_CMD.Entry_Mode_Set, 0x07,
                            (byte)GraphicDriverCommandType.Command, 5, (byte)ILI9341_CMD.Display_Function_Control, 0x0A, 0x82, 0x27, 0x00,
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.Sleep_Out,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.Normal_Display_On,
                            // Sleep 10 ms
                            (byte)GraphicDriverCommandType.Sleep, 1,
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.Display_ON,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.NOP,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,

                        },
                        OrientationLandscape = new byte[]
                        {
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.NOP,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                            (byte)GraphicDriverCommandType.Command, 2, (byte)ILI9341_CMD.Memory_Access_Control, (byte)(ILI9341_Orientation.MADCTL_MV | ILI9341_Orientation.MADCTL_MX | ILI9341_Orientation.MADCTL_BGR),
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.Memory_Write,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                        },
                        OrientationLandscape180 = new byte[]
                        {
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.NOP,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                            (byte)GraphicDriverCommandType.Command, 2, (byte)ILI9341_CMD.Memory_Access_Control, (byte)(ILI9341_Orientation.MADCTL_MY | ILI9341_Orientation.MADCTL_MV | ILI9341_Orientation.MADCTL_BGR),
                             // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.Memory_Write,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,                       },
                        OrientationPortrait = new byte[]
                        {
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.NOP,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                            (byte)GraphicDriverCommandType.Command, 2, (byte)ILI9341_CMD.Memory_Access_Control, (byte)(ILI9341_Orientation.MADCTL_MX | ILI9341_Orientation.MADCTL_BGR),
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.Memory_Write,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,                        },
                        OrientationPortrait180 = new byte[]
                        {
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.NOP,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                            (byte)GraphicDriverCommandType.Command, 2, (byte)ILI9341_CMD.Memory_Access_Control, (byte)(ILI9341_Orientation.MADCTL_MY | ILI9341_Orientation.MADCTL_BGR),
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.Memory_Write,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                        },
                        PowerModeNormal = new byte[]
                        {
                            (byte)GraphicDriverCommandType.Command, 3, (byte)ILI9341_CMD.POWER_STATE, 0x00, 0x00,
                        },
                        PowerModeSleep = new byte[]
                        {
                            (byte)GraphicDriverCommandType.Command, 3, (byte)ILI9341_CMD.POWER_STATE, 0x00, 0x01,
                        },
                        DefaultOrientation = DisplayOrientation.Landscape,
                        Brightness = (byte)ILI9341_CMD.Write_Display_Brightness,
                        SetWindowType = SetWindowType.X16bitsY16Bit,
                    };
                }

                return _driver;
            }
        }

        /// <summary>
        /// Gets the graphic driver without adjusting the manufacturer default settings for the Ili9341 display.
        /// Use this driver when you are unsure first.
        /// </summary>
        public static GraphicDriver GraphicDriverWithDefaultManufacturingSettings
        {
            get
            {
                if (_driverDefaultSettings == null)
                {
                    _driverDefaultSettings = new GraphicDriver()
                    {
                        BitsPerPixel = 16,
                        MemoryWrite = (byte)ILI9341_CMD.Memory_Write,
                        SetColumnAddress = (byte)ILI9341_CMD.Column_Address_Set,
                        SetRowAddress = (byte)ILI9341_CMD.Page_Address_Set,
                        InitializationSequence = new byte[]
                        {
                            (byte)GraphicDriverCommandType.Command, 2, (byte)ILI9341_CMD.Interface_Signal_Control, 0xE0,
                            (byte)GraphicDriverCommandType.Command, 4, (byte)ILI9341_CMD.Interface_Control, 0x01, 0x00, 0X00,
                            (byte)GraphicDriverCommandType.Command, 2, (byte)ILI9341_CMD.Pixel_Format_Set, 0x55,
                            (byte)GraphicDriverCommandType.Command, 5, (byte)ILI9341_CMD.Display_Function_Control, 0x08, 0x82, (byte)((Height / 8) - 1), 0x04,
                            (byte)GraphicDriverCommandType.Command, 5, (byte)ILI9341_CMD.Column_Address_Set, 0, 0, (byte)((Height - 1) >> 8), (byte)((Height - 1) & 0xFF),
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.Sleep_Out,
                            // Sleep 10 ms
                            (byte)GraphicDriverCommandType.Sleep, 1,
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.Display_ON,
                            // Sleep 200 ms
                            (byte)GraphicDriverCommandType.Sleep, 20,
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.NOP,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,

                        },
                        OrientationLandscape = new byte[]
                        {
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.NOP,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                            (byte)GraphicDriverCommandType.Command, 2, (byte)ILI9341_CMD.Memory_Access_Control, (byte)(ILI9341_Orientation.MADCTL_MV  | ILI9341_Orientation.MADCTL_MX | ILI9341_Orientation.MADCTL_BGR),
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.Memory_Write,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                        },
                        OrientationLandscape180 = new byte[]
                        {
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.NOP,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                            (byte)GraphicDriverCommandType.Command, 2, (byte)ILI9341_CMD.Memory_Access_Control, (byte)(ILI9341_Orientation.MADCTL_MY | ILI9341_Orientation.MADCTL_BGR),
                             // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.Memory_Write,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,                       },
                        OrientationPortrait = new byte[]
                        {
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.NOP,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                            (byte)GraphicDriverCommandType.Command, 2, (byte)ILI9341_CMD.Memory_Access_Control, (byte)(ILI9341_Orientation.MADCTL_MX | ILI9341_Orientation.MADCTL_BGR),
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.Memory_Write,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,                        },
                        OrientationPortrait180 = new byte[]
                        {
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.NOP,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                            (byte)GraphicDriverCommandType.Command, 2, (byte)ILI9341_CMD.Memory_Access_Control, (byte)(ILI9341_Orientation.MADCTL_MY | ILI9341_Orientation.MADCTL_BGR),
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,
                            (byte)GraphicDriverCommandType.Command, 1, (byte)ILI9341_CMD.Memory_Write,
                            // Sleep 20 ms
                            (byte)GraphicDriverCommandType.Sleep, 2,                        },
                        PowerModeNormal = new byte[]
                        {
                            (byte)GraphicDriverCommandType.Command, 3, (byte)ILI9341_CMD.POWER_STATE, 0x00, 0x00,
                        },
                        PowerModeSleep = new byte[]
                        {
                            (byte)GraphicDriverCommandType.Command, 3, (byte)ILI9341_CMD.POWER_STATE, 0x00, 0x01,
                        },
                        DefaultOrientation = DisplayOrientation.Portrait,
                        Brightness = (byte)ILI9341_CMD.Write_Display_Brightness,
                        SetWindowType = SetWindowType.X16bitsY16Bit,
                    };
                }

                return _driverDefaultSettings;
            }
        }
    }
}
