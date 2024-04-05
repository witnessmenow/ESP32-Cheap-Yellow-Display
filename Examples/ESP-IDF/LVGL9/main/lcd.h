#pragma once

#include <esp_err.h>
#include <lvgl.h>
#include <esp_lvgl_port.h>

esp_err_t lcd_display_brightness_init(void);
esp_err_t lcd_display_brightness_set(int );
esp_err_t lcd_display_backlight_off(void);
esp_err_t lcd_display_backlight_on(void);
esp_err_t lcd_display_rotate(lv_display_t *, lv_display_rotation_t );

esp_err_t app_lcd_init(esp_lcd_panel_io_handle_t *, esp_lcd_panel_handle_t *);
lv_display_t* app_lvgl_init(esp_lcd_panel_io_handle_t , esp_lcd_panel_handle_t );