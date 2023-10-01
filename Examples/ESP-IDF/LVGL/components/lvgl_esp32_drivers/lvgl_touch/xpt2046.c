/**
 * @file XPT2046.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "xpt2046.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "tp_spi.h"
#include <stddef.h>

/*********************
 *      DEFINES
 *********************/
#define TAG "XPT2046"

#define CMD_X_READ  0b10010000		//0x90
#define CMD_Y_READ  0b11010000		//0xD0

/**********************
 *      TYPEDEFS
 * 起点在左上角 0，0
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void xpt2046_corr(int16_t * x, int16_t * y);
static void xpt2046_avg(int16_t * x, int16_t * y);

/**********************
 *  STATIC VARIABLES
 **********************/
int16_t avg_buf_x[XPT2046_AVG];
int16_t avg_buf_y[XPT2046_AVG];
uint8_t avg_last;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

//初始化 XPT2046
void xpt2046_init(void)
{
	// 配置XPT2046的中断引脚
	gpio_config_t irq_config = {
		.pin_bit_mask = BIT64(XPT2046_IRQ),
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE,
	};
	esp_err_t ret = gpio_config(&irq_config);
	// 配置XPT2046的数据输出（MISO）引脚
	gpio_config_t miso_config = {
		.pin_bit_mask = BIT64(XPT2046_MISO),
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE,
	};
	ret = gpio_config(&miso_config);
	// 配置其它数据脚
	esp_rom_gpio_pad_select_gpio(XPT2046_MOSI);
	gpio_set_direction(XPT2046_MOSI, GPIO_MODE_OUTPUT);// 设置GPIO为推挽输出模式
	esp_rom_gpio_pad_select_gpio(XPT2046_CLK);
	gpio_set_direction(XPT2046_CLK, GPIO_MODE_OUTPUT);// 设置GPIO为推挽输出模式
	esp_rom_gpio_pad_select_gpio(XPT2046_CS);
	gpio_set_direction(XPT2046_CS, GPIO_MODE_OUTPUT);// 设置GPIO为推挽输出模式
	
	ESP_LOGI(TAG,"XPT2046 Initialization");

	assert(ret == ESP_OK);
}

/**
 * Get the current position and state of the touchpad
 * @param data store the read data here
 * @return false: because no more data to be read
 */
bool xpt2046_read(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
	static int16_t last_x = 0,last_y = 0;
	bool valid = true;
	int16_t x = 0,y = 0;
	uint16_t ux = 0,uy = 0;
	uint8_t irq = gpio_get_level(XPT2046_IRQ);
	if (irq == 0) {
		uint8_t data[2];
		if(TP_Read_XY2(&ux,&uy)==1){
			ESP_LOGI(TAG,"XPT2046 Read 原始值：x:%d   y:%d", ux, uy);
			if(ux > 350){ux -= 350;}else{ux = 0;}
			if(uy > 190){uy -= 190;}else{uy = 0;}
			ux = (uint32_t)((uint32_t)ux * LV_HOR_RES) / (3870 - 350);//320   3870-350
			uy = (uint32_t)((uint32_t)uy * LV_VER_RES) / (3870 - 190);// 240  3870-190
			// 翻转坐标
			//ux =  LV_HOR_RES - ux;
			ux=ux + 20;
			//uy =  LV_VER_RES - uy;
			ESP_LOGI(TAG,"XPT2046 Read 坐标值：x:%d   y:%d", ux, uy);
			x = ux;
			y = uy;
			last_x = ux;
			last_y = uy;
		}else{
			ESP_LOGI(TAG,"XPT2046 Read Error");
			x = last_x;
			y = last_y;
			avg_last = 0;
			valid = false;
		}
	} else {
		x = last_x;
		y = last_y;
		avg_last = 0;
		valid = false;
	}
	data->point.x = x;
	data->point.y = y;
	data->state = valid == false ? LV_INDEV_STATE_REL : LV_INDEV_STATE_PR;
	return false;
}
uint16_t TP_Read_XOY(uint8_t xy)
{
	uint8_t READ_TIMES = 30;//读取次数
	uint8_t LOST_VAL = 1;//丢弃值
	uint16_t i, j,temp,buf[READ_TIMES];
	uint32_t sum=0;
	for(i=0;i<READ_TIMES;i++){
		buf[i]=xpt2046_gpio_spi_read_reg(xy);
	}
	for(i=0;i<READ_TIMES-1; i++){//排序
		for(j=i+1;j<READ_TIMES;j++){
			if(buf[i]>buf[j]){//升序排列
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}
	sum=0;
	for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++){
		sum+=buf[i];
	}
	temp = sum/(READ_TIMES-2*LOST_VAL);
	return temp;
}

//读取x,y坐标
//最小值不能少于100.
//x,y:读取到的坐标值
//返回值:0,失败;1,成功。
uint8_t TP_Read_XY(uint16_t *x,uint16_t *y)
{
	uint16_t xtemp,ytemp;
	xtemp=TP_Read_XOY(CMD_X_READ);
	ytemp=TP_Read_XOY(CMD_Y_READ);
	//if(xtemp<100||ytemp<100)return 0;//读数失败
	*x=xtemp;
	*y=ytemp;
	return 1;//读数成功
}
//连续2次读取触摸屏IC,且这两次的偏差不能超过
//ERR_RANGE,满足条件,则认为读数正确,否则读数错误
//该函数能大大提高准确度
//x,y:读取到的坐标值
//返回值:0,失败;1,成功。
#define ERR_RANGE 50 //误差范围 
uint8_t TP_Read_XY2(uint16_t *x,uint16_t *y)
{
	uint16_t x1,y1,x2,y2;
	uint8_t flag;
	flag=TP_Read_XY(&x1,&y1);
	if(flag==0)return(0);
	flag=TP_Read_XY(&x2,&y2);
	if(flag==0)return(0);   
	if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//前后两次采样在+-50内
	&&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
	{
		*x=(x1+x2)/2;
		*y=(y1+y2)/2;
		return 1;
	}else return 0;
}

// 向XPT2016写入一个字节
void xpt2046_gpio_Write_Byte(uint8_t data)
{
	uint8_t i=0;
	for(i=0;i<8;i++){
		if(data&0x80){
			gpio_set_level(XPT2046_MOSI, 1);
		}else{
			gpio_set_level(XPT2046_MOSI, 0);
		}  
		data<<=1; 
		gpio_set_level(XPT2046_CLK, 0);
		gpio_set_level(XPT2046_CLK, 1);
	}
}

uint16_t xpt2046_gpio_spi_read_reg(uint8_t reg)
{
	uint8_t count=0;
	uint16_t Data=0; 
	gpio_set_level(XPT2046_CLK, 0);		// 先拉低时钟
	gpio_set_level(XPT2046_MOSI, 0); 	// 拉低数据线
	gpio_set_level(XPT2046_CS, 0); 		// 选中触摸屏IC

	xpt2046_gpio_Write_Byte(reg);		// 发送命令字

	esp_rom_delay_us(6);					// ADS7846的转换时间最长为6us

	gpio_set_level(XPT2046_CLK, 0);
	esp_rom_delay_us(1);
	gpio_set_level(XPT2046_CLK, 1);			// 给1个时钟，清除BUSY
	gpio_set_level(XPT2046_CLK, 0);
	for(count=0;count<16;count++){			// 读出16位数据,只有高12位有效
		Data<<=1; 	 
		gpio_set_level(XPT2046_CLK, 0);		// 下降沿有效
		gpio_set_level(XPT2046_CLK, 1);	
		if(gpio_get_level(XPT2046_MISO))Data++;
	}
	Data>>=4;								// 只有高12位有效
	gpio_set_level(XPT2046_CS, 1);			// 释放片选
	return(Data);
}
/**********************
 *   STATIC FUNCTIONS
 **********************/
//xpt2046_corr(&x, &y);
static void xpt2046_corr(int16_t * x, int16_t * y)
{
#if XPT2046_XY_SWAP != 0
	int16_t swap_tmp;
    swap_tmp = *x;
    *x = *y;
    *y = swap_tmp;
#endif
	if((*x) > XPT2046_X_MIN)
		(*x) -= XPT2046_X_MIN;
	else
		(*x) = 0;

	if((*y) > XPT2046_Y_MIN)(*y) -= XPT2046_Y_MIN;
	else(*y) = 0;

	(*x) = (uint32_t)((uint32_t)(*x) * LV_HOR_RES) / (XPT2046_X_MAX - XPT2046_X_MIN);
	(*y) = (uint32_t)((uint32_t)(*y) * LV_VER_RES) / (XPT2046_Y_MAX - XPT2046_Y_MIN);

#if XPT2046_X_INV != 0
	(*x) =  LV_HOR_RES - (*x);
#endif

#if XPT2046_Y_INV != 0
	(*y) =  LV_VER_RES - (*y);
#endif
}

//xpt2046_avg(&x, &y);
static void xpt2046_avg(int16_t * x, int16_t * y)
{
	/*Shift out the oldest data*/
	uint8_t i;
	for(i = XPT2046_AVG - 1; i > 0 ; i--) {
		avg_buf_x[i] = avg_buf_x[i - 1];
		avg_buf_y[i] = avg_buf_y[i - 1];
	}

	/*Insert the new point*/
	avg_buf_x[0] = *x;
	avg_buf_y[0] = *y;
	if(avg_last < XPT2046_AVG) avg_last++;

	/*Sum the x and y coordinates*/
	int32_t x_sum = 0;
	int32_t y_sum = 0;
	for(i = 0; i < avg_last ; i++) {
		x_sum += avg_buf_x[i];
		y_sum += avg_buf_y[i];
	}

	/*Normalize the sums*/
	(*x) = (int32_t)x_sum / avg_last;
	(*y) = (int32_t)y_sum / avg_last;
}
