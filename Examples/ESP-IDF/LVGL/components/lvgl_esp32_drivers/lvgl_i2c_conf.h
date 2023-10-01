/**
 * @file lvgl_i2c_config.h
 */

#ifndef LVGL_I2C_CONF_H
#define LVGL_I2C_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/

/* TODO: Define the I2C bus clock based on the selected display or touch
 * controllers. */

/* Do both display and touch controllers uses I2C? */
#if defined (CONFIG_LVGL_TOUCH_DRIVER_PROTOCOL_I2C) &&     \
    defined (CONFIG_LVGL_TFT_DISPLAY_PROTOCOL_I2C)

#if defined (CONFIG_LVGL_DISPLAY_I2C_PORT_0) && \
    defined (CONFIG_LVGL_TOUCH_I2C_PORT_0)
#define SHARED_I2C_PORT
#define DISP_I2C_PORT    I2C_NUM_0
#endif

#if defined (CONFIG_LVGL_DISPLAY_I2C_PORT_1) && \
    defined (CONFIG_LVGL_TOUCH_I2C_PORT_1)
#define SHARED_I2C_PORT
#define DISP_I2C_PORT    I2C_NUM_1
#endif

#if !defined (SHARED_I2C_PORT)
#endif
#endif

#if defined (SHARED_I2C_PORT)
/* If the port is shared the display and touch controllers must use the same
 * SCL and SDA pins, otherwise let the user know with an error. */
#if (CONFIG_LVGL_DISP_PIN_SDA != CONFIG_LVGL_TOUCH_I2C_SDA) || \
    (CONFIG_LVGL_DISP_PIN_SCL != CONFIG_LVGL_TOUCH_I2C_SCL)
#error "To share I2C port you need to choose the same SDA and SCL pins on both display and touch configurations"
#endif

#define DISP_I2C_SDA            CONFIG_LVGL_DISP_PIN_SDA
#define DISP_I2C_SCL            CONFIG_LVGL_DISP_PIN_SCL
#define DISP_I2C_ORIENTATION    TFT_ORIENTATION_LANDSCAPE

/* Setting the I2C speed to the slowest one */
#if DISP_I2C_SPEED_HZ < TOUCH_I2C_SPEED_HZ
#define DISP_I2C_SPEED_HZ       400000 /* DISP_I2C_SPEED_HZ */
#else
#define DISP_I2C_SPEED_HZ       400000 /* DISP_I2C_SPEED_HZ */
#endif

#else

/* lets check if the touch controller uses I2C... */
#if defined (CONFIG_LVGL_TOUCH_DRIVER_PROTOCOL_I2C)
#if defined (CONFIG_LVGL_TOUCH_I2C_PORT_0)
#define TOUCH_I2C_PORT           I2C_NUM_0
#else
#define TOUCH_I2C_PORT           I2C_NUM_1
#endif
#define TOUCH_I2C_SDA            CONFIG_LVGL_TOUCH_I2C_SDA
#define TOUCH_I2C_SCL            CONFIG_LVGL_TOUCH_I2C_SCL
#define TOUCH_I2C_SPEED_HZ       400000
#endif

/* lets check if the display controller uses I2C... */
#if defined (CONFIG_LVGL_TFT_DISPLAY_PROTOCOL_I2C)
#if defined (CONFIG_LVGL_DISPLAY_I2C_PORT_0)
#define DISP_I2C_PORT           I2C_NUM_0
#else
#define DISP_I2C_PORT           I2C_NUM_1
#endif

#define DISP_I2C_SDA            CONFIG_LVGL_DISP_PIN_SDA
#define DISP_I2C_SCL            CONFIG_LVGL_DISP_PIN_SCL
#define DISP_I2C_ORIENTATION    TFT_ORIENTATION_LANDSCAPE
#define DISP_I2C_SPEED_HZ       400000
#endif

#endif

/**********************
 *      TYPEDEFS
 **********************/


/**********************
 * GLOBAL PROTOTYPES
 **********************/


/**********************
 *      MACROS
 **********************/


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LVGL_I2C_CONF_H*/
