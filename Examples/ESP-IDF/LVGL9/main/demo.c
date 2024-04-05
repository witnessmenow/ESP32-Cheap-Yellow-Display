#include <stdio.h>
#include <math.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <freertos/semphr.h>

#include <esp_system.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_check.h>

#include <lvgl.h>
#include <esp_lvgl_port.h>

#include "lcd.h"
#include "touch.h"

static const char *TAG="demo";

lv_obj_t *lbl_counter;

void ui_event_Screen(lv_event_t *e)
{
static uint8_t pos=1;

    lv_event_code_t event_code = lv_event_get_code(e);
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_user_data(e);

    if (event_code == LV_EVENT_CLICKED)
    {
        lv_obj_align(btn, pos++, 0, 0);
        if (pos > 9) pos=1;
    }
}


static esp_err_t app_lvgl_main(void)
{
    lv_obj_t *scr = lv_scr_act();

    lvgl_port_lock(0);

    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "Hello LVGL 9 and esp_lvgl_port!");
    lv_obj_set_style_text_color(label, lv_color_white(), LV_STATE_DEFAULT);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -48);

    lv_obj_t *labelR = lv_label_create(scr);
    lv_label_set_text(labelR, "Red");
    lv_obj_set_style_text_color(labelR, lv_color_make(0xff, 0, 0), LV_STATE_DEFAULT);
    lv_obj_align(labelR, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *labelG = lv_label_create(scr);
    lv_label_set_text(labelG, "Green");
    lv_obj_set_style_text_color(labelG, lv_color_make(0, 0xff, 0), LV_STATE_DEFAULT);
    lv_obj_align(labelG, LV_ALIGN_TOP_MID, 0, 32);

    lv_obj_t *labelB = lv_label_create(scr);
    lv_label_set_text(labelB, "Blue");
    lv_obj_set_style_text_color(labelB, lv_color_make(0, 0, 0xff), LV_STATE_DEFAULT);
    lv_obj_align(labelB, LV_ALIGN_TOP_MID, 0, 64);

    lv_obj_t *btn_counter = lv_button_create(scr);
    lv_obj_align(btn_counter, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(btn_counter, 120, 50);
    lv_obj_add_event_cb(btn_counter, ui_event_Screen, LV_EVENT_ALL, btn_counter);

    lbl_counter = lv_label_create(btn_counter);
    lv_label_set_text(lbl_counter, "testing");
    lv_obj_set_style_text_color(lbl_counter, lv_color_make(248, 11, 181), LV_STATE_DEFAULT);
    lv_obj_align(lbl_counter, LV_ALIGN_CENTER, 0, 0);

    lvgl_port_unlock();

    return ESP_OK;
}


void app_main(void)
{
esp_lcd_panel_io_handle_t lcd_io;
esp_lcd_panel_handle_t lcd_panel;
esp_lcd_touch_handle_t tp;
lvgl_port_touch_cfg_t touch_cfg;
lv_display_t *lvgl_display = NULL;
char buf[16];
uint16_t n = 0;

    ESP_ERROR_CHECK(lcd_display_brightness_init());

    ESP_ERROR_CHECK(app_lcd_init(&lcd_io, &lcd_panel));
    lvgl_display = app_lvgl_init(lcd_io, lcd_panel);
    if (lvgl_display == NULL)
    {
        ESP_LOGI(TAG, "fatal error in app_lvgl_init");
        esp_restart();
    }
    
    ESP_ERROR_CHECK(touch_init(&tp));
    touch_cfg.disp = lvgl_display;
    touch_cfg.handle = tp;
    lvgl_port_add_touch(&touch_cfg);

    ESP_ERROR_CHECK(lcd_display_brightness_set(75));
    ESP_ERROR_CHECK(lcd_display_rotate(lvgl_display, LV_DISPLAY_ROTATION_90));
    ESP_ERROR_CHECK(app_lvgl_main());

    while(42)
    {
        sprintf(buf, "%04d", n++);

        if (lvgl_port_lock(0))
        {
            lv_label_set_text(lbl_counter, buf);

            lvgl_port_unlock();
        }

        vTaskDelay(125 / portTICK_PERIOD_MS);
    }
    vTaskDelay(portMAX_DELAY);
}
