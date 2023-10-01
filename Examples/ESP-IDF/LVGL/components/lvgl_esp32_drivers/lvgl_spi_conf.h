/**
 * @file lvgl_spi_conf.h
 *
 */

#ifndef LVGL_SPI_CONF_H
#define LVGL_SPI_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/
// DISPLAY PINS
#define DISP_SPI_MOSI CONFIG_LVGL_DISP_SPI_MOSI
#if defined (CONFIG_LVGL_DISPLAY_USE_SPI_MISO)
    #define DISP_SPI_MISO CONFIG_LVGL_DISP_SPI_MISO
    #define DISP_SPI_INPUT_DELAY_NS CONFIG_LVGL_DISP_SPI_INPUT_DELAY_NS
#else
    #define DISP_SPI_MISO (-1)
    #define DISP_SPI_INPUT_DELAY_NS (0)
#endif
#define DISP_SPI_CLK CONFIG_LVGL_DISP_SPI_CLK
#if defined (CONFIG_LVGL_DISPLAY_USE_SPI_CS)
    #define DISP_SPI_CS CONFIG_LVGL_DISP_SPI_CS
#else
    #define DISP_SPI_CS (-1)
#endif

/* Define TOUCHPAD PINS when selecting a touch controller */
#if !defined (CONFIG_LVGL_TOUCH_CONTROLLER_NONE)

/* Handle FT81X special case */
#if defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_FT81X) && \
    defined (CONFIG_LVGL_TOUCH_CONTROLLER_FT81X)

    #define TP_SPI_MOSI CONFIG_LVGL_DISP_SPI_MOSI
    #define TP_SPI_MISO CONFIG_LVGL_DISP_SPI_MISO
    #define TP_SPI_CLK  CONFIG_LVGL_DISP_SPI_CLK
    #define TP_SPI_CS   CONFIG_LVGL_DISP_SPI_CS
#else
    #define TP_SPI_MOSI CONFIG_LVGL_TOUCH_SPI_MOSI
    #define TP_SPI_MISO CONFIG_LVGL_TOUCH_SPI_MISO
    #define TP_SPI_CLK  CONFIG_LVGL_TOUCH_SPI_CLK
    #define TP_SPI_CS   CONFIG_LVGL_TOUCH_SPI_CS
#endif
#endif

#define ENABLE_TOUCH_INPUT  CONFIG_LVGL_ENABLE_TOUCH

#if CONFIG_LVGL_TFT_DISPLAY_SPI_HSPI == 1
#define TFT_SPI_HOST HSPI_HOST
#else
#define TFT_SPI_HOST VSPI_HOST
#endif /*CONFIG_LVGL_TFT_DISPLAY_SPI_HSPI == 1*/

#if defined (CONFIG_LVGL_TOUCH_CONTROLLER_SPI_HSPI)
#define TOUCH_SPI_HOST HSPI_HOST
#elif defined (CONFIG_LVGL_TOUCH_CONTROLLER_SPI_VSPI)
#define TOUCH_SPI_HOST VSPI_HOST
#endif /*CONFIG_LVGL_TOUCH_CONTROLLER_SPI_HSPI == 1*/

/* Handle the FT81X Special case */
#if defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_FT81X)

#if defined (CONFIG_LVGL_TOUCH_CONTROLLER_FT81X)
#define SHARED_SPI_BUS
#else
/* Empty */
#endif

#else
// Detect the use of a shared SPI Bus and verify the user specified the same SPI bus for both touch and tft
#if defined (CONFIG_LVGL_TOUCH_DRIVER_PROTOCOL_SPI) && TP_SPI_MOSI == DISP_SPI_MOSI && TP_SPI_CLK == DISP_SPI_CLK
#if TFT_SPI_HOST != TOUCH_SPI_HOST
#error You must specify the same SPI host (HSPI or VSPI) for both display and touch driver
#endif

#define SHARED_SPI_BUS
#endif

#endif

/**********************
 *      TYPEDEFS
 **********************/
#if defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_ILI9481) || \
    defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_ILI9488)

#define SPI_BUS_MAX_TRANSFER_SZ (DISP_BUF_SIZE * 3)

#elif defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_ILI9341)  || \
      defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_ST7789)   || \
      defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_ST7735S)  || \
      defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_HX8357)   || \
      defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_SH1107)   || \
      defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_FT81X)    || \
      defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_IL3820)

#define SPI_BUS_MAX_TRANSFER_SZ (DISP_BUF_SIZE * 2)

#else
#define SPI_BUS_MAX_TRANSFER_SZ (DISP_BUF_SIZE * 2)
#endif

#if defined (CONFIG_LVGL_TFT_USE_CUSTOM_SPI_CLK_DIVIDER)
#define SPI_TFT_CLOCK_SPEED_HZ ((80 * 1000 * 1000) / CONFIG_LVGL_TFT_CUSTOM_SPI_CLK_DIVIDER)
#else
#if defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_ST7789)
#define SPI_TFT_CLOCK_SPEED_HZ  (20*1000*1000)
#elif defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_ST7735S)
#define SPI_TFT_CLOCK_SPEED_HZ  (40*1000*1000)
#elif defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_HX8357)
#define SPI_TFT_CLOCK_SPEED_HZ  (26*1000*1000)
#elif defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_SH1107)
#define SPI_TFT_CLOCK_SPEED_HZ  (8*1000*1000)
#elif defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_ILI9481)
#define SPI_TFT_CLOCK_SPEED_HZ  (16*1000*1000)
#elif defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_ILI9486)
#define SPI_TFT_CLOCK_SPEED_HZ  (20*1000*1000)
#elif defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_ILI9488)
#define SPI_TFT_CLOCK_SPEED_HZ  (40*1000*1000)
#elif defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_ILI9341)
#define SPI_TFT_CLOCK_SPEED_HZ  (40*1000*1000)
#elif defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_FT81X)
#define SPI_TFT_CLOCK_SPEED_HZ  (32*1000*1000)
#else
#define SPI_TFT_CLOCK_SPEED_HZ  (40*1000*1000)
#endif

#endif


#if defined (CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_ST7789)
#define SPI_TFT_SPI_MODE    (2)
#else
#define SPI_TFT_SPI_MODE    (0)
#endif

/* Touch driver */
#if (CONFIG_LVGL_TOUCH_CONTROLLER == TOUCH_CONTROLLER_STMPE610)
#define SPI_TOUCH_CLOCK_SPEED_HZ    (1*1000*1000)
#define SPI_TOUCH_SPI_MODE          (1)
#else
#define SPI_TOUCH_CLOCK_SPEED_HZ    (2*1000*1000)
#define SPI_TOUCH_SPI_MODE          (0)
#endif

/**********************
 * GLOBAL PROTOTYPES
 **********************/


/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LVGL_SPI_CONF_H*/
