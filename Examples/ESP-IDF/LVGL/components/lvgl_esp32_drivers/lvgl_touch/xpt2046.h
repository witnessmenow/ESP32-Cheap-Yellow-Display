// XPT2046.h
#ifndef XPT2046_H
#define XPT2046_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include <stdbool.h>
#include "../../lvgl/lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/
#define XPT2046_IRQ			36
#define XPT2046_MOSI		32
#define XPT2046_MISO		39
#define XPT2046_CLK			25
#define XPT2046_CS			33

#define XPT2046_AVG			4
#define XPT2046_X_MIN		200
#define XPT2046_Y_MIN		120
#define XPT2046_X_MAX		1900
#define XPT2046_Y_MAX		1900
#define XPT2046_X_INV		1		// X翻转
#define XPT2046_Y_INV		1		// Y翻转
#define XPT2046_XY_SWAP		0

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void xpt2046_init(void);
bool xpt2046_read(lv_indev_drv_t * drv, lv_indev_data_t * data);
uint16_t xpt2046_gpio_spi_read_reg(uint8_t reg);
void xpt2046_gpio_Write_Byte(uint8_t data);		// 向XPT2016写入一个字节
uint16_t TP_Read_XOY(uint8_t xy);


uint8_t TP_Read_XY(uint16_t *x,uint16_t *y);
uint8_t TP_Read_XY2(uint16_t *x,uint16_t *y);
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XPT2046_H */
