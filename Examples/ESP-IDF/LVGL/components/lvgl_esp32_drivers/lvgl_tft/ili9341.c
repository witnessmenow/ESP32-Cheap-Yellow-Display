/**
 * @file ili9341.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "ili9341.h"
#include "disp_spi.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*********************
 *      DEFINES
 *********************/
 #define TAG "ILI9341"

/**********************
 *      TYPEDEFS
 **********************/

/*The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct. */
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void ili9341_set_orientation(uint8_t orientation);

static void ili9341_send_cmd(uint8_t cmd);
static void ili9341_send_data(void * data, uint16_t length);
static void ili9341_send_color(void * data, uint16_t length);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void ili9341_init(void)
{
	lcd_init_cmd_t ili_init_cmds[]={
		{0xCF, {0x00, 0xc1, 0X30}, 3},                          //
		{0xED, {0x64, 0x03, 0X12, 0X81}, 4},                    //
		{0xE8, {0x85, 0x10, 0x7a}, 3},                          //
		{0xCB, {0x39, 0x2C, 0x00, 0x34, 0x02}, 5},              //
		//{0xF7, {0x20}, 1},
		{0xEA, {0x00, 0x00}, 2},                               //
		{0xC0, {0x1b}, 1},          /*Power control*/          //
		{0xC1, {0x00}, 1},          /*Power control */         //
		{0xC2, {0x11}, 1},          /*Power control */         //
		{0xC5, {0x30, 0x30}, 2},    /*VCOM control*/           //
		{0xC7, {0xb7}, 1},          /*VCOM control*/           //
		{0x36, {0x08}, 1},          /*Memory Access Control*/  //08
		{0x3A, {0x55}, 1},			/*Pixel Format Set*/       //
		{0xB1, {0x00, 0x1a}, 2},                               //
		{0xB6, {0x0A, 0xe6, 0x27, 0x002}, 4},                               //Display Function Control 
		{0xF2, {0x00}, 1},                                     //
		{0xF7, {0x20}, 1},                                     //
		{0xF1, {0x01,0x31}, 2},                                //
		{0x26, {0x01}, 1},                                     //
		{0xE0, {0x0f, 0x2a,0x28, 0x08, 0x0e, 0x08, 0x54, 0xa9, 0X43, 0x0a, 0x0f, 0x00, 0x00, 0x00, 0x00}, 15},   //
		{0XE1, {0x00, 0x15,0x17, 0x07, 0x11, 0x06, 0x2b, 0x56, 0x3c, 0x05, 0x10, 0x0f, 0x3f, 0x3f, 0x0F}, 15},   //
		{0x2A, {0x00, 0x00, 0x00, 0x7F}, 4},
		{0x2B, {0x00, 0x00, 0x00, 0xa0}, 4},
		{0x2C, {0}, 0},
		//{0xB7, {0x07}, 1},
		//{0xB6, {0x0A, 0xa2}, 2},                             //
		{0x11, {0}, 0x80},
		{0x29, {0}, 0x80},
		{0, {0}, 0xff},
	};

#if ILI9341_BCKL == 15
	gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_SEL_15;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
#endif

	//Initialize non-SPI GPIOs
	gpio_set_direction(ILI9341_DC, GPIO_MODE_OUTPUT);
	gpio_set_direction(ILI9341_RST, GPIO_MODE_OUTPUT);

#if ILI9341_ENABLE_BACKLIGHT_CONTROL
    gpio_set_direction(ILI9341_BCKL, GPIO_MODE_OUTPUT);
#endif
	//Reset the display
	gpio_set_level(ILI9341_RST, 0);
	vTaskDelay(pdMS_TO_TICKS(100));
	gpio_set_level(ILI9341_RST, 1);
	vTaskDelay(pdMS_TO_TICKS(100));
	printf("%s->ili9341 Initialization....\n",TAG);
	//Send all the commands
	uint16_t cmd = 0;
	while (ili_init_cmds[cmd].databytes!=0xff) {
		ili9341_send_cmd(ili_init_cmds[cmd].cmd);
		ili9341_send_data(ili_init_cmds[cmd].data, ili_init_cmds[cmd].databytes&0x1F);
		if (ili_init_cmds[cmd].databytes & 0x80) {
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		cmd++;
	}

	ili9341_enable_backlight(true);
    ili9341_set_orientation(CONFIG_LVGL_DISPLAY_ORIENTATION);

#if ILI9341_INVERT_COLORS == 1
	ili9341_send_cmd(0x21);
#else
	ili9341_send_cmd(0x20);
#endif
}


void ili9341_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_map)
{
	uint8_t data[4];

	/*Column addresses*/
	ili9341_send_cmd(0x2A);
	data[0] = (area->x1 >> 8) & 0xFF;
	data[1] = area->x1 & 0xFF;
	data[2] = (area->x2 >> 8) & 0xFF;
	data[3] = area->x2 & 0xFF;
	ili9341_send_data(data, 4);

	/*Page addresses*/
	ili9341_send_cmd(0x2B);
	data[0] = (area->y1 >> 8) & 0xFF;
	data[1] = area->y1 & 0xFF;
	data[2] = (area->y2 >> 8) & 0xFF;
	data[3] = area->y2 & 0xFF;
	ili9341_send_data(data, 4);

	/*Memory write*/
	ili9341_send_cmd(0x2C);


	uint32_t size = lv_area_get_width(area) * lv_area_get_height(area);

	ili9341_send_color((void*)color_map, size * 2);
}

void ili9341_enable_backlight(bool backlight)
{
#if ILI9341_ENABLE_BACKLIGHT_CONTROL
    ESP_LOGI(TAG, "%s backlight.", backlight ? "Enabling" : "Disabling");
    uint32_t tmp = 0;

#if (ILI9341_BCKL_ACTIVE_LVL==1)
    tmp = backlight ? 1 : 0;
#else
    tmp = backlight ? 0 : 1;
#endif

    gpio_set_level(ILI9341_BCKL, tmp);
#endif
}

void ili9341_sleep_in()
{
	uint8_t data[] = {0x08};
	ili9341_send_cmd(0x10);
	ili9341_send_data(&data, 1);
}

void ili9341_sleep_out()
{
	uint8_t data[] = {0x08};
	ili9341_send_cmd(0x11);
	ili9341_send_data(&data, 1);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/


static void ili9341_send_cmd(uint8_t cmd)
{
    disp_wait_for_pending_transactions();
    gpio_set_level(ILI9341_DC, 0);	 /*Command mode*/
    disp_spi_send_data(&cmd, 1);
}

static void ili9341_send_data(void * data, uint16_t length)
{
    disp_wait_for_pending_transactions();
    gpio_set_level(ILI9341_DC, 1);	 /*Data mode*/
    disp_spi_send_data(data, length);
}

static void ili9341_send_color(void * data, uint16_t length)
{
    disp_wait_for_pending_transactions();
    gpio_set_level(ILI9341_DC, 1);   /*Data mode*/
    disp_spi_send_colors(data, length);
}

// 设置屏幕方向
static void ili9341_set_orientation(uint8_t orientation)
{
	// ESP_ASSERT(orientation < 4);
	const char *orientation_str[] = {"PORTRAIT", "PORTRAIT_INVERTED", "LANDSCAPE", "LANDSCAPE_INVERTED"};
	printf("%s->Display orientation: %s\n",TAG, orientation_str[orientation]);
#if defined CONFIG_LVGL_PREDEFINED_DISPLAY_M5STACK
	uint8_t data[] = {0x68, 0x68, 0x08, 0x08};
#elif defined (CONFIG_LVGL_PREDEFINED_DISPLAY_WROVER4)
	uint8_t data[] = {0x4C, 0x88, 0x28, 0xE8};
#elif defined (CONFIG_LVGL_PREDEFINED_DISPLAY_NONE)
	uint8_t data[] = {0x48, 0x88, 0x28, 0xE8};
#endif
	printf("%s->0x36 command value: 0x%02X\n",TAG, data[orientation]);
	ili9341_send_cmd(0x36);
	ili9341_send_data((void *) &data[orientation], 1);
}
