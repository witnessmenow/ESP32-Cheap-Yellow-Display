/* LVGL Example project

液晶驱动芯片支持
ILI9341 240*320 3.2     14-25FPS
ILI9488 320*480 3.5     3-7FPS

ENC28J60_INT	GPIO34
ENC28J60_CS		GPIO5
FLASH_CS		GPIO21
SD_CS			GPIO22

VSPI SPI3_HOST=2	SD、FALASH、ENC28J60
CS0		GPIO_5
SCLK	GPIO_18
MISO	GPIO_19
MOSI	GPIO_23
QUADWP	GPIO_22
QUADHD	GPIO_21

HSPI SPI2_HOST=1   液晶独用
CS0		GPIO_15
SCLK	GPIO_14
MISO	GPIO_12
MOSI	GPIO_13
QUADWP	GPIO_2
QUADHD	GPIO_4


液晶SPI引脚定义  HSPI    SPI2_HOST=1
omponent config → LVGL TFT Display controller → Display Pin Assignments
MOSI (Master Out Slave In)			GPIO_13
MISO (Slave Out Master In)			None		GPIO_12
CLK (SCK / Serial Clock)			GPIO_14
CS (Slave Select)					GPIO_15
DC (Data / Command)					GPIO_2
Reset								GOIO_4
Backlight							None

触摸SPI引脚定义components\lvgl_esp32_drivers\lvgl_touch\xpt2046.h
omponent config → LVGL Touch controller → Touchpanel (XPT2046) Pin Assignments
MISO (Master In Slave Out)			GPIO_39(SENSOR_VN只能输入)
MOSI (Master Out Slave In)			GPIO_32
CLK (SCK / Serial Clock)			GPIO_33
CS (Slave Select)					GPIO_25
IRQ (Interrupt Request)				GPIO_36(SENSOR_VP只能输入)      


==================================================模块左==================================================
3V3			3V3			2		供电
复位按钮	 EN			 3		高电平：芯片使能/低电平：芯片关闭/注意：不能让 EN 管脚浮空
TP_IRQ		SENSOR_VP	4		GPIO36, ADC1_CH0, RTC_GPIO0			只能输入
TP_MISO		SENSOR_VN	5		GPIO39, ADC1_CH3, RTC_GPIO3			只能输入
RLight		IO34		6		GPIO34, ADC1_CH6, RTC_GPIO4			只能输入
IRRev		IO35		7		GPIO35, ADC1_CH7, RTC_GPIO5			只能输入
TP_MOSI		IO32		8		GPIO32, XTAL_32K_P (32.768 kHz 晶振输入), ADC1_CH4, TOUCH9,RTC_GPIO9
TP_CS		IO33		9		GPIO33, XTAL_32K_N (32.768 kHz 晶振输出), ADC1_CH5, TOUCH8,RTC_GPIO8
TP_CLK		IO25		10		GPIO25, DAC_1, ADC2_CH8, RTC_GPIO6, EMAC_RXD0
DHT11		IO26		11		GPIO26, DAC_2, ADC2_CH9, RTC_GPIO7, EMAC_RXD1
TouchPad	IO27		12		GPIO27, ADC2_CH7, TOUCH7, RTC_GPIO17, EMAC_RX_DV
TFT_CLK		IO14		13		GPIO14, ADC2_CH6, TOUCH6, RTC_GPIO16, MTMS, HSPICLK,HS2_CLK, SD_CLK, EMAC_TXD2
TFT_MISO	IO12		14		GPIO12, ADC2_CH5, TOUCH5, RTC_GPIO15, MTDI, HSPIQ,HS2_DATA2, SD_DATA2, EMAC_TXD3

================================================模块下==================================================
GND			GND			15		接地
TFT_MOSI	IO13		16		GPIO13, ADC2_CH4, TOUCH4, RTC_GPIO14, MTCK, HSPID,HS2_DATA3, SD_DATA3, EMAC_RX_ER
XXXX		SD2			17		NC
XXXX		SD3			18		NC
XXXX		CMD			19		NC
XXXX		CLK			20		NC
XXXX		SD0			21		NC
XXXX		SD1			22		NC
TFT_CS		IO15		23		GPIO15, ADC2_CH3, TOUCH3, MTDO, HSPICS0, RTC_GPIO13,HS2_CMD, SD_CMD, EMAC_RXD3
TFT_DC		IO2			24		GPIO2, ADC2_CH2, TOUCH2, RTC_GPIO12, HSPIWP, HS2_DATA0,SD_DATA0

================================================模块右==================================================
GND			GND			38		接地
SD_MOSI		IO23		37		GPIO23, VSPID, HS1_STROBE
IIC_SDA		IO22		36		GPIO22, VSPIWP, U0RTS, EMAC_TXD1
UART_RXD	TXD0		35		GPIO1, U0TXD, CLK_OUT3, EMAC_RXD2
UART_TXD	RXD0		34		GPIO3, U0RXD, CLK_OUT2
IIC_SCL		IO21		33		GPIO21, VSPIHD, EMAC_TX_EN
XXX			NC			32		NC
SD_MISO		IO19		31		GPIO19, VSPIQ, U0CTS, EMAC_TXD0
SD_CLK		IO18		30		GPIO18, VSPICLK, HS1_DATA7
SD_CS		IO5			29		GPIO5, VSPICS0, HS1_DATA6, EMAC_RX_CLK
IRSend		IO17		28		GPIO17, HS1_DATA5, U2TXD, EMAC_CLK_OUT_180
RGBLight	IO16		27		GPIO16, HS1_DATA4, U2RXD, EMAC_CLK_OUT
TFT_Reset	IO4			26		GPIO4, ADC2_CH0, TOUCH0, RTC_GPIO10, HSPIHD, HS2_DATA1,SD_DATA1, EMAC_TX_ER
Flash按钮	IO0			25		GPIO0, ADC2_CH1, TOUCH1, RTC_GPIO11, CLK_OUT1,EMAC_TX_CLK

 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

// Littlevgl 头文件
#include "lvgl/lvgl.h"			// LVGL头文件
#include "lvgl_helpers.h"		// 助手 硬件驱动相关

#include "lv_examples/src/lv_demo_widgets/lv_demo_widgets.h"




//LV_IMG_DECLARE(mouse_cursor_icon);			/*Declare the image file.*/


/*********************
 *      DEFINES
 *********************/
#define TAG " LittlevGL Demo"
#define LV_TICK_PERIOD_MS 10

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_tick_task(void *arg);
void guiTask(void *pvParameter);				// GUI任务


// 主函数
void app_main() {
	printf("\r\nAPP %s is start!~\r\n", TAG);
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	// 如果要使用任务创建图形，则需要创建固定核心任务,否则可能会出现诸如内存损坏等问题
	// 创建一个固定到其中一个核心的FreeRTOS任务，选择核心1
	xTaskCreatePinnedToCore(guiTask, "gui", 4096*2, NULL, 0, NULL, 1);
}

static void lv_tick_task(void *arg) {
    (void) arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

//Creates a semaphore to handle concurrent call to lvgl stuff
//If you wish to call *any* lvgl function from other threads/tasks
//you should lock on the very same semaphore!
SemaphoreHandle_t xGuiSemaphore;		// 创建一个GUI信号量

void guiTask(void *pvParameter) {
    
    (void) pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();    // 创建GUI信号量
    lv_init();          // 初始化LittlevGL
    lvgl_driver_init(); // 初始化液晶SPI驱动 触摸芯片SPI/IIC驱动

    static lv_color_t buf1[DISP_BUF_SIZE];
#ifndef CONFIG_LVGL_TFT_DISPLAY_MONOCHROME
    static lv_color_t buf2[DISP_BUF_SIZE];
#endif
    static lv_disp_buf_t disp_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;

#if defined CONFIG_LVGL_TFT_DISPLAY_CONTROLLER_IL3820 
    /* Actual size in pixel, not bytes and use single buffer */
    size_in_px *= 8;
    lv_disp_buf_init(&disp_buf, buf1, NULL, size_in_px);
#elif defined CONFIG_LVGL_TFT_DISPLAY_MONOCHROME
    lv_disp_buf_init(&disp_buf, buf1, NULL, size_in_px);
#else
    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);
#endif

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;

// 如果配置为 单色模式
#ifdef CONFIG_LVGL_TFT_DISPLAY_MONOCHROME
    disp_drv.rounder_cb = disp_driver_rounder;
    disp_drv.set_px_cb = disp_driver_set_px;
#endif

    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);


// 如果有配置触摸芯片，配置触摸
#if CONFIG_LVGL_TOUCH_CONTROLLER != TOUCH_CONTROLLER_NONE
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touch_driver_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);
#endif


    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    // 一个标签演示
    //lv_obj_t * scr = lv_disp_get_scr_act(NULL);         // 获取当前屏幕
    //lv_obj_t * label1 =  lv_label_create(scr, NULL);    // 在当前活动的屏幕上创建标签
    //lv_label_set_text(label1, "Hello\nworld!");         // 修改标签的文字
    // 对象对齐函数，将标签中心对齐，NULL表示在父级上对齐，当前父级是屏幕，0，0表示对齐后的x，y偏移量
    //lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, 0);
/*
	lv_indev_t * mouse_indev = lv_indev_drv_register(&indev_drv);
	lv_obj_t * cursor_obj =  lv_img_create(lv_scr_act(), NULL); //Create an image object for the cursor 
	lv_img_set_src(cursor_obj, &mouse_cursor_icon);             //Set the image source
	lv_indev_set_cursor(mouse_indev, cursor_obj);               //Connect the image  object to the driver
*/	
	lv_demo_widgets();
	
    while (1) {
		vTaskDelay(1);
		// 尝试锁定信号量，如果成功，请调用lvgl的东西
		if (xSemaphoreTake(xGuiSemaphore, (TickType_t)10) == pdTRUE) {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);  // 释放信号量
        }
    }
    vTaskDelete(NULL);      // 删除任务
}


