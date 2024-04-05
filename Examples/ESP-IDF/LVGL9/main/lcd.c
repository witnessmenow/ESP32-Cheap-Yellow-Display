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
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_ops.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <driver/spi_master.h>
#include <esp_lcd_ili9341.h>

#include <lvgl.h>
#include <esp_lvgl_port.h>

#include "hardware.h"

static const char *TAG="lcd";

esp_err_t lcd_display_brightness_init(void)
{
    const ledc_channel_config_t LCD_backlight_channel = {
        .gpio_num = LCD_BACKLIGHT,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LCD_BACKLIGHT_LEDC_CH,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = 1,
        .duty = 0,
        .hpoint = 0,
        .flags.output_invert = false
    };
 
    const ledc_timer_config_t LCD_backlight_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = 1,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&LCD_backlight_timer));
    ESP_ERROR_CHECK(ledc_channel_config(&LCD_backlight_channel));
 
    return ESP_OK;
}

esp_err_t lcd_display_brightness_set(int brightness_percent)
{
    if (brightness_percent > 100) {
        brightness_percent = 100;
    }
    if (brightness_percent < 0) {
        brightness_percent = 0;
    }

    ESP_LOGI(TAG, "Setting LCD backlight: %d%%", brightness_percent);

    uint32_t duty_cycle = (1023 * brightness_percent) / 100;

    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LCD_BACKLIGHT_LEDC_CH, duty_cycle));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LCD_BACKLIGHT_LEDC_CH));

    return ESP_OK;
}

esp_err_t lcd_display_backlight_off(void)
{
    return lcd_display_brightness_set(0);
}

esp_err_t lcd_display_backlight_on(void)
{
    return lcd_display_brightness_set(100);
}

esp_err_t lcd_display_rotate(lv_display_t *lvgl_disp, lv_display_rotation_t dir)
{
    if (lvgl_disp)
    {
        lv_display_set_rotation(lvgl_disp, dir);
        return ESP_OK;
    }

    return ESP_FAIL;
}

esp_err_t app_lcd_init(esp_lcd_panel_io_handle_t *lcd_io, esp_lcd_panel_handle_t *lcd_panel)
{
    const spi_bus_config_t buscfg = { 
        .mosi_io_num = LCD_SPI_MOSI,
        .miso_io_num = LCD_SPI_MISO,
        .sclk_io_num = LCD_SPI_CLK,
        .quadhd_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .max_transfer_sz = LCD_DRAWBUF_SIZE * sizeof(uint16_t),
    };

    ESP_RETURN_ON_ERROR(spi_bus_initialize(LCD_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO), TAG, "SPI init failed");


    const esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = LCD_CS,
        .dc_gpio_num = LCD_DC,
        .spi_mode = 0,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
    };

    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_HOST, &io_config, lcd_io), TAG, "LCD new panel io failed");


    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RESET,
        .color_space = ESP_LCD_COLOR_SPACE_BGR,
        .bits_per_pixel = LCD_BITS_PIXEL,
    };

    esp_err_t r = esp_lcd_new_panel_ili9341(*lcd_io, &panel_config, lcd_panel);

    esp_lcd_panel_reset(*lcd_panel);
    esp_lcd_panel_init(*lcd_panel);

    esp_lcd_panel_mirror(*lcd_panel, LCD_MIRROR_X, LCD_MIRROR_Y);
    esp_lcd_panel_disp_on_off(*lcd_panel, true);

    return r;
}


lv_display_t *app_lvgl_init(esp_lcd_panel_io_handle_t lcd_io, esp_lcd_panel_handle_t lcd_panel)
{
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,
        .task_stack = 4096,
        .task_affinity = -1,
        .task_max_sleep_ms = 500,
        .timer_period_ms = 5
    };

    esp_err_t e = lvgl_port_init(&lvgl_cfg);

    if (e != ESP_OK)
    {
        ESP_LOGI(TAG, "lvgl_port_init() failed: %s", esp_err_to_name(e));

        return NULL;
    }


    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd_io,
        .panel_handle = lcd_panel,
        .buffer_size = LCD_DRAWBUF_SIZE,
        .double_buffer = LCD_DOUBLE_BUFFER,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .rotation = {
            .swap_xy = false,
            .mirror_x = LCD_MIRROR_X,
            .mirror_y = LCD_MIRROR_Y,
        },
        .flags = {
            .buff_dma = true,
            .buff_spiram = false,
            .swap_bytes = true,
        }
    };
    
    return lvgl_port_add_disp(&disp_cfg);
}
