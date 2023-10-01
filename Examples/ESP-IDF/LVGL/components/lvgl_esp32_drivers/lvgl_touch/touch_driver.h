/**
 * @file touch_driver.h
 */

#ifndef TOUCH_DRIVER_H
#define TOUCH_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include <stdbool.h>
#include "../../lvgl/lvgl/lvgl.h"

#if defined (CONFIG_LVGL_TOUCH_CONTROLLER_XPT2046)
#include "xpt2046.h"
#elif defined (CONFIG_LVGL_TOUCH_CONTROLLER_FT6X06)
#include "ft6x36.h"
#elif defined (CONFIG_LVGL_TOUCH_CONTROLLER_STMPE610)
#include "stmpe610.h"
#elif defined (CONFIG_LVGL_TOUCH_CONTROLLER_ADCRAW)
#include "adcraw.h"
#elif defined (CONFIG_LVGL_TOUCH_CONTROLLER_FT81X)
#include "FT81x.h"
#elif defined (CONFIG_LVGL_TOUCH_CONTROLLER_RA8875)
#include "ra8875_touch.h"
#endif

/*********************
*      DEFINES
*********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void touch_driver_init(void);
bool touch_driver_read(lv_indev_drv_t *drv, lv_indev_data_t *data);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TOUCH_DRIVER_H */

