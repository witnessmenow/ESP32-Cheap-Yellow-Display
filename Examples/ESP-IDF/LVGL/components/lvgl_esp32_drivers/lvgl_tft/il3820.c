/**
@file il3820.c
@brief   Waveshare e-paper 2.9in b/w display
@version 1.0
@date    2020-05-29
@author  Juergen Kienhoefer


@section LICENSE

MIT License

Copyright (c) 2020 Juergen Kienhoefer

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 */

/*********************
 *      INCLUDES
 *********************/
#include "disp_spi.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "il3820.h"

/*********************
 *      DEFINES
 *********************/
 #define TAG "IL3820"

/**
 * SSD1673, SSD1608 compatible EPD controller driver.
 */

#define BIT_SET(a,b) ((a) |= (1U<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1U<<(b)))

#define EPD_PANEL_WIDTH			CONFIG_LVGL_DISPLAY_WIDTH
#define EPD_PANEL_HEIGHT		CONFIG_LVGL_DISPLAY_HEIGHT
#define EPD_PANEL_NUMOF_COLUMS		EPD_PANEL_WIDTH
#define EPD_PANEL_NUMOF_ROWS_PER_PAGE	8
#define EPD_PANEL_NUMOF_PAGES		(EPD_PANEL_HEIGHT / EPD_PANEL_NUMOF_ROWS_PER_PAGE)

#define IL3820_PANEL_FIRST_PAGE	0
#define IL3820_PANEL_LAST_PAGE		(EPD_PANEL_NUMOF_PAGES - 1)
#define IL3820_PANEL_FIRST_GATE	0
#define IL3820_PANEL_LAST_GATE		(EPD_PANEL_NUMOF_COLUMS - 1)

#define IL3820_PIXELS_PER_BYTE		8

const uint8_t il3820_scan_mode = IL3820_DATA_ENTRY_XIYIY;

static uint8_t il3820_lut_initial[] = {
			0x50, 0xAA, 0x55, 0xAA, 0x11, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x1F, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static uint8_t il3820_lut_default[] = {
			0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 
			0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
			0x00, 0x00, 0x13, 0x14, 0x44, 0x12,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t il3820_softstart[] = {0xd7, 0xd6, 0x9d};
static uint8_t il3820_vcom[] = {0xa8};
static uint8_t il3820_dummyline[] = {0x1a}; // 4 dummy lines per gate
static uint8_t il3820_gatetime[] = {0x08}; // 2us per line
static uint8_t il3820_border[] = {0x03};

static bool il3820_partial = false;


static void il3820_waitbusy( int wait_ms )
{
    int i;
    vTaskDelay( 10 / portTICK_RATE_MS); // 10ms delay
	for( i=0; i<(wait_ms*10); i++ ){
		if( gpio_get_level(IL3820_BUSY_PIN) != IL3820_BUSY_LEVEL ) {
            //ESP_LOGI( TAG, "busy %dms", i*10 );
            return;
        }
		vTaskDelay(10 / portTICK_RATE_MS);
	}
	ESP_LOGE( TAG, "busy exceeded %dms", i*10 );
}



static inline void il3820_write_cmd( uint8_t cmd, const uint8_t *data, size_t len) 
{
	disp_wait_for_pending_transactions();
	gpio_set_level(IL3820_DC_PIN, 0);  // command mode
	disp_spi_send_data(&cmd, 1);

	if (data != NULL) {
		gpio_set_level(IL3820_DC_PIN, 1); // data mode
		disp_spi_send_data( data, len);
	}
}

static inline void il3820_send_cmd( uint8_t cmd ) 
{
	disp_wait_for_pending_transactions();
	gpio_set_level(IL3820_DC_PIN, 0);  // command mode
	disp_spi_send_data( &cmd, 1 );
}


static void il3820_send_data( const uint8_t *data, uint16_t length )
{
    disp_wait_for_pending_transactions();
	gpio_set_level( IL3820_DC_PIN, 1) ;  // data mode
    disp_spi_send_colors( data, length ); // requires the flush
}


static inline void il3820_set_window( uint16_t sx, uint16_t ex, uint16_t ys, uint16_t ye)
{
	uint8_t tmp[4];
	
	tmp[0] = sx / 8;
	tmp[1] = ex / 8;
	il3820_write_cmd( IL3820_CMD_RAM_XPOS_CTRL, tmp, 2 ); // set X address start/end

	tmp[0] = ys % 256;
    tmp[1] = ys / 256;
    tmp[2] = ye % 256;
    tmp[3] = ye / 256;
	il3820_write_cmd( IL3820_CMD_RAM_YPOS_CTRL, tmp,	4 );
}

static inline void il3820_set_cursor( uint16_t sx, uint16_t ys )
{
	uint8_t tmp[2];

	tmp[0] = sx / 8;
	il3820_write_cmd( IL3820_CMD_RAM_XPOS_CNTR, tmp, 1 ); // set X address counter

	tmp[0] = ys % 256;
    tmp[1] = ys / 256;
	il3820_write_cmd( IL3820_CMD_RAM_YPOS_CNTR, tmp, 2 );
}


// used by write
static void il3820_update_display(  void )
{
	uint8_t tmp;

	//ESP_LOGI(TAG, "update partial %d", il3820_partial );
	if( il3820_partial ) {
		tmp = IL3820_CTRL2_TO_PATTERN;
	} else {
		tmp = (IL3820_CTRL2_ENABLE_CLK |
	       	IL3820_CTRL2_ENABLE_ANALOG |
	       	IL3820_CTRL2_TO_PATTERN );
	       	//IL3820_CTRL2_DISABLE_ANALOG |
	       	//IL3820_CTRL2_DISABLE_CLK);
	}
	il3820_write_cmd( IL3820_CMD_UPDATE_CTRL2, &tmp, 1 );

	il3820_write_cmd( IL3820_CMD_MASTER_ACTIVATION, NULL, 0);
	il3820_waitbusy( IL3820_WAIT );
	il3820_write_cmd( IL3820_CMD_TERMINATE_FRAME_RW, NULL, 0);
}



// used by init
static void il3820_clear_cntlr_mem(uint8_t ram_cmd, bool update)
{
	// DMA buffers in the stack is still allowed but externl ram enable SPIRAM_ALLOW_STACK_EXTERNAL_MEMORY
	WORD_ALIGNED_ATTR uint8_t clear_page[ EPD_PANEL_WIDTH/8 ]; // used by SPI, must be word alligned

    uint16_t Height = EPD_PANEL_HEIGHT;
	ESP_LOGI(TAG, "clear" );

	il3820_write_cmd( IL3820_CMD_ENTRY_MODE, &il3820_scan_mode, 1);

	il3820_set_window( 0, EPD_PANEL_WIDTH-1, 0, EPD_PANEL_HEIGHT-1 ); // start/end
	//il3820_set_cursor( 0, 0 );

	memset(clear_page, 0xff, sizeof(clear_page));
	for( int j = 0; j < Height; j++ ){
		il3820_set_cursor( 0, j );
		il3820_write_cmd( ram_cmd, clear_page, sizeof(clear_page));
	}

	if (update) {
		il3820_set_window( 0, EPD_PANEL_WIDTH-1, 0, EPD_PANEL_HEIGHT-1 );
		il3820_update_display();
	}
}



void il3820_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
	size_t linelen = (area->x2 - area->x1 + 1)/8;
    uint8_t *buffer = (uint8_t*)color_map;
	// skip lines
	size_t address = (area->y1 * IL3820_COLUMNS) + (area->x1/8);

	ESP_LOGI(TAG, "flush: %d,%d at %d,%d", area->x1, area->x2, area->y1, area->y2 );

	il3820_write_cmd( IL3820_CMD_ENTRY_MODE, &il3820_scan_mode, 1);
	il3820_set_window( area->x1, area->x2, area->y1, area->y2);
	il3820_set_cursor( area->x1, area->y1);

	il3820_send_cmd( IL3820_CMD_WRITE_RAM );
    for(size_t row = area->y1; row <= area->y2; row++){
		//printf("from 0x%p, length 0x%x\n", buffer+address, linelen );
		//il3820_set_cursor( area->x1, row );
		//il3820_send_cmd( IL3820_CMD_WRITE_RAM );
		il3820_send_data( buffer + address, linelen ); // reverses bits in byte
		buffer += IL3820_COLUMNS; // next line down
    }

	il3820_set_window( area->x1, area->x2, area->y1, area->y2);
	il3820_update_display();
	/* IMPORTANT!!!
     * Inform the graphics library that you are ready with the flushing */
	lv_disp_flush_ready(drv);

}


void il3820_set_px_cb(struct _disp_drv_t * disp_drv, uint8_t* buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y,
        lv_color_t color, lv_opa_t opa) {

	/* buf_w will be ignored, the configured CONFIG_LVGL_DISPLAY_HEIGHT and _WIDTH,
	   and CONFIG_LVGL_DISPLAY_ORIENTATION_LANDSCAPE and _PORTRAIT will be used. */ 		
    uint16_t byte_index = 0;
    uint8_t  bit_index = 0;

	byte_index = y + (( x>>3 ) * CONFIG_LVGL_DISPLAY_HEIGHT);
	bit_index  = x & 0x7;

    if ( color.full != 0 ) {
        BIT_SET(buf[byte_index], 7 - bit_index);
    } else {
        BIT_CLEAR(buf[byte_index], 7 - bit_index);
    }
}



void il3820_rounder(struct _disp_drv_t * disp_drv, lv_area_t *area) {
    area->x1 = area->x1 & ~(0x7);
    area->x2 = area->x2 |  (0x7);
}


void il3820_sleep_in(void) {
	il3820_waitbusy( IL3820_WAIT );
	uint8_t data[] = {0x01};
	il3820_write_cmd( IL3820_CMD_SLEEP_MODE, data, 1);
}



// main initialize
void il3820_init( void )
{
	uint8_t tmp[3];

	ESP_LOGI(TAG, "init");
	
	//Initialize non-SPI GPIOs
    gpio_set_direction(IL3820_DC_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(IL3820_RST_PIN, GPIO_MODE_OUTPUT);
	gpio_set_direction(IL3820_BUSY_PIN,  GPIO_MODE_INPUT);

	gpio_set_level( IL3820_RST_PIN, 0);
	vTaskDelay( IL3820_RESET_DELAY / portTICK_RATE_MS );
	gpio_set_level( IL3820_RST_PIN, 1);
	vTaskDelay( IL3820_RESET_DELAY / portTICK_RATE_MS );

	il3820_write_cmd(IL3820_CMD_SW_RESET, NULL, 0); //already hardware reset
	il3820_waitbusy( IL3820_WAIT );

	tmp[0] = ( EPD_PANEL_HEIGHT - 1) & 0xFF;
	tmp[1] = ( EPD_PANEL_HEIGHT >> 8 );
	tmp[2] = 0; // GD = 0; SM = 0; TB = 0;
	il3820_write_cmd( IL3820_CMD_GDO_CTRL, tmp, 3);

	il3820_write_cmd( IL3820_CMD_SOFTSTART, il3820_softstart, sizeof(il3820_softstart));

	il3820_write_cmd( IL3820_CMD_VCOM_VOLTAGE, il3820_vcom, 1);

	il3820_write_cmd( IL3820_CMD_DUMMY_LINE, il3820_dummyline, 1);

	il3820_write_cmd( IL3820_CMD_GATE_LINE_WIDTH, il3820_gatetime, 1);

	il3820_write_cmd( IL3820_CMD_BWF_CTRL, il3820_border, 1);

	il3820_write_cmd( IL3820_CMD_UPDATE_LUT, il3820_lut_initial, sizeof(il3820_lut_initial));
	il3820_clear_cntlr_mem( IL3820_CMD_WRITE_RAM, true);
	//il3820_clear_cntlr_mem( IL3820_CMD_WRITE_RED_RAM, false);

	// allow partial updates now
	il3820_partial = true;

	il3820_write_cmd( IL3820_CMD_UPDATE_LUT, il3820_lut_default, sizeof(il3820_lut_default));
	il3820_clear_cntlr_mem( IL3820_CMD_WRITE_RAM, true);
}




