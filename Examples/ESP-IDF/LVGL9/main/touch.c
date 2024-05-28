#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <esp_log.h>
#include <esp_err.h>
#include <esp_timer.h>
#include <esp_lcd_touch.h>
#include <esp_lcd_touch_xpt2046.h>

#include <driver/spi_master.h>
#include <driver/gpio.h>

#include "hardware.h"
#include "touch.h"

static uint16_t map(uint16_t n, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max)
{
    uint16_t value = (n - in_min) * (out_max - out_min) / (in_max - in_min);
    return (value < out_min) ? out_min : ((value > out_max) ? out_max : value);
}

static void process_coordinates(esp_lcd_touch_handle_t tp, uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
    *x = map(*x, TOUCH_X_RES_MIN, TOUCH_X_RES_MAX, 0, LCD_H_RES);
    *y = map(*y, TOUCH_Y_RES_MIN, TOUCH_Y_RES_MAX, 0, LCD_V_RES);
}

esp_err_t touch_init(esp_lcd_touch_handle_t *tp)
{
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;

    const esp_lcd_panel_io_spi_config_t tp_io_config = { .cs_gpio_num = TOUCH_CS,
        .dc_gpio_num = TOUCH_DC,
        .spi_mode = 0,
        .pclk_hz = TOUCH_CLOCK_HZ,
        .trans_queue_depth = 3,
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .flags = { .dc_low_on_data = 0, .octal_mode = 0, .sio_mode = 0, .lsb_first = 0, .cs_high_active = 0 } };

    static const int SPI_MAX_TRANSFER_SIZE = 32768;
    const spi_bus_config_t buscfg_touch = { .mosi_io_num = TOUCH_SPI_MOSI,
        .miso_io_num = TOUCH_SPI_MISO,
        .sclk_io_num = TOUCH_SPI_CLK,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .data4_io_num = GPIO_NUM_NC,
        .data5_io_num = GPIO_NUM_NC,
        .data6_io_num = GPIO_NUM_NC,
        .data7_io_num = GPIO_NUM_NC,
        .max_transfer_sz = SPI_MAX_TRANSFER_SIZE,
        .flags = SPICOMMON_BUSFLAG_SCLK | SPICOMMON_BUSFLAG_MISO | SPICOMMON_BUSFLAG_MOSI | SPICOMMON_BUSFLAG_MASTER | SPICOMMON_BUSFLAG_GPIO_PINS,
        .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
        .intr_flags = ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_IRAM };

    esp_lcd_touch_config_t tp_cfg = {.x_max = LCD_H_RES,
                                   .y_max = LCD_V_RES,
                                   .rst_gpio_num = TOUCH_RST,
                                   .int_gpio_num = TOUCH_IRQ,
                                   .levels = {.reset = 0, .interrupt = 0},
                                   .flags =
                                       {
                                           .swap_xy = false,
                                           .mirror_x = LCD_MIRROR_X,
                                           .mirror_y = LCD_MIRROR_Y,
                                       },
                                   .process_coordinates = process_coordinates,
                                   .interrupt_callback = NULL};

    ESP_ERROR_CHECK(spi_bus_initialize(TOUCH_SPI, &buscfg_touch, SPI_DMA_CH_AUTO));

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)TOUCH_SPI, &tp_io_config, &tp_io_handle));
    ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(tp_io_handle, &tp_cfg, tp));

    return ESP_OK;
}