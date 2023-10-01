/**
 * @file disp_driver.c
 */

#include "disp_driver.h"
#include "disp_spi.h"

void disp_driver_init(void)
{
    ili9341_init();
    //ili9481_init();
    //ili9488_init();
    //st7789_init();
    //st7735s_init();
    //ili9486_init();
    //sh1107_init();
    //ssd1306_init();
}

void disp_driver_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map)
{
    ili9341_flush(drv, area, color_map);
    //ili9481_flush(drv, area, color_map);
    //ili9488_flush(drv, area, color_map);
    //st7789_flush(drv, area, color_map);
    //st7735s_flush(drv, area, color_map);
    //ili9486_flush(drv, area, color_map);
    //sh1107_flush(drv, area, color_map);
    //ssd1306_flush(drv, area, color_map);

}

void disp_driver_rounder(lv_disp_drv_t * disp_drv, lv_area_t * area)
{
#if defined CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_SSD1306
    ssd1306_rounder(disp_drv, area);
#elif defined CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_SH1107
    sh1107_rounder(disp_drv, area);
#elif defined CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_IL3820
    il3820_rounder(disp_drv, area);
#endif
}

void disp_driver_set_px(lv_disp_drv_t * disp_drv, uint8_t * buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y,
    lv_color_t color, lv_opa_t opa) 
{
#if defined CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_SSD1306
    ssd1306_set_px_cb(disp_drv, buf, buf_w, x, y, color, opa);
#elif defined CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_SH1107
    sh1107_set_px_cb(disp_drv, buf, buf_w, x, y, color, opa);
#elif defined CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_IL3820
    il3820_set_px_cb(disp_drv, buf, buf_w, x, y, color, opa);
#endif
}
