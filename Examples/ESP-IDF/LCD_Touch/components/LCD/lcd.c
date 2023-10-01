#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stdint.h"
#include "stdbool.h"
#include "driver/gpio.h"
#include "lcd.h"
#include "math.h"  
#include "string.h"

static const char *TAG = "lcd";

/*The LCD needs a bunch of command/argument values to be initialized. They are stored in this struct. */
typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;
spi_device_handle_t LCD_SPI_Handle;

uint16_t LCD_Width = 240;		// LCD 宽度
uint16_t LCD_Height = 320;		// LCD 高度
uint8_t  LCD_Orientation = LCD_DISPLAY_ORIENTATION_PORTRAIT;// 默认纵向
// 初始化液晶
void Init_LCD(uint16_t color)
{
	int cmd=0;
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
		{0x36, {0x08}, 1},          /*Memory Access Control*/  //
		{0x3A, {0x55}, 1},			/*Pixel Format Set*/       //
		{0xB1, {0x00, 0x1a}, 2},                               //
		//{0xB6, {0x0A, 0x86, 0x27, 0x002}, 4},                               //Display Function Control 
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
		{0xB6, {0x0A, 0xa2}, 2},                             ////Display Function Control
		{0x11, {0}, 0x80},
		{0x29, {0}, 0x80},
		{0, {0}, 0xff},
	};
	esp_err_t ret;
	// SPI总线配置
	spi_bus_config_t buscfg = {
		.miso_io_num = PIN_NUM_MISO,
		.mosi_io_num = PIN_NUM_MOSI,
		.sclk_io_num = PIN_NUM_CLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz=PARALLEL_LINES*320*2+8
	};
	// SPI驱动接口配置
	spi_device_interface_config_t devcfg = {
		.clock_speed_hz = 30*1000*1000,				// SPI时钟 30 MHz
		.mode = 0,									// SPI模式0
		.spics_io_num = PIN_NUM_CS,					// CS片选信号引脚
		.queue_size = 7,							// 事务队列尺寸 7个
		.pre_cb = lcd_spi_pre_transfer_callback,	// 数据传输前回调，用作D/C（数据命令）线分别处理
	};
	// 初始化SPI总线
	ret=spi_bus_initialize(LCD_HOST, &buscfg, DMA_CHAN);
	ESP_ERROR_CHECK(ret);
	// 添加SPI总线驱动
	ret=spi_bus_add_device(LCD_HOST, &devcfg, &LCD_SPI_Handle);
	ESP_ERROR_CHECK(ret);

	// 初始化其它控制引脚
	gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
	//gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
	gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

	// Reset the display
	//gpio_set_level(PIN_NUM_RST, 0);
	//vTaskDelay(pdMS_TO_TICKS(100));
	//gpio_set_level(PIN_NUM_RST, 1);
	//vTaskDelay(pdMS_TO_TICKS(100));
	gpio_set_level(PIN_NUM_BCKL, 1);

	uint32_t lcd_id = LCD_Get_ID();
	printf("LCD ID: %08lX\n", lcd_id);

	printf("LCD ILI9341 initialization.\n");

	// 循环发送设置所有寄存器
	while (ili_init_cmds[cmd].databytes!=0xff) {
		LCD_WriteCMD(ili_init_cmds[cmd].cmd);
		LCD_WriteDate(ili_init_cmds[cmd].data, ili_init_cmds[cmd].databytes&0x1F);
		if (ili_init_cmds[cmd].databytes&0x80) {
			vTaskDelay(pdMS_TO_TICKS(100));
		}
		cmd++;
	}
	LCD_Set_Orientation(LCD_DISPLAY_ORIENTATION_PORTRAIT);// 纵向
	//LCD_WriteCMD(0x21);// 翻转颜色
	LCD_WriteCMD(0x20);
	LCD_Clear(color);
	// gpio_set_level(PIN_NUM_BCKL, 0);	// 点亮LCD屏
}

// 发送命令到LCD，使用轮询方式阻塞等待传输完成。
// 由于数据传输量很少，因此在轮询方式处理可提高速度。使用中断方式的开销要超过轮询方式。
void LCD_WriteCMD(const uint8_t cmd)
{
	esp_err_t ret;
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));		// 清空结构体
	t.length=8;						// 要传输的位数 一个字节 8位
	t.tx_buffer=&cmd;				// 将命令填充进去
	t.user=(void*)0;				// 设置D/C 线，在SPI传输前回调中根据此值处理DC信号线
	ret=spi_device_polling_transmit(LCD_SPI_Handle, &t);		// 开始传输
	assert(ret==ESP_OK);			// 一般不会有问题
}

// 发送数据到LCD，使用轮询方式阻塞等待传输完成。
// 由于数据传输量很少，因此在轮询方式处理可提高速度。使用中断方式的开销要超过轮询方式。
void LCD_WriteDate(const uint8_t *data, int len)
{
	esp_err_t ret;
	spi_transaction_t t;
	if (len==0) return;				// 长度为0 没有数据要传输
	memset(&t, 0, sizeof(t));		// 清空结构体
	t.length=len*8;					// 要写入的数据长度 Len 是字节数，len, transaction length is in bits.
	t.tx_buffer=data;				// 数据指针
	t.user=(void*)1;				// 设置D/C 线，在SPI传输前回调中根据此值处理DC信号线
	ret=spi_device_polling_transmit(LCD_SPI_Handle, &t);		// 开始传输
	assert(ret==ESP_OK);			// 一般不会有问题
}
// 发送数据到LCD，使用轮询方式阻塞等待传输完成。
// 由于数据传输量很少，因此在轮询方式处理可提高速度。使用中断方式的开销要超过轮询方式。
void LCD_WriteDate16(uint16_t data)
{
	esp_err_t ret;
	spi_transaction_t t;
	uint8_t dataBuf[2] = {0,0};
	dataBuf[0] = data>>8;
	dataBuf[1] = data&0xFF;
	memset(&t, 0, sizeof(t));		// 清空结构体
	t.length=2*8;					// 要写入的数据长度 Len 是字节数，len, transaction length is in bits.
	t.tx_buffer=dataBuf;				// 数据指针
	t.user=(void*)1;				// 设置D/C 线，在SPI传输前回调中根据此值处理DC信号线
	ret=spi_device_polling_transmit(LCD_SPI_Handle, &t);		// 开始传输
	assert(ret==ESP_OK);			// 一般不会有问题
}

// 此函数在SPI传输开始之前被调用（在irq上下文中！），通过用户字段的值来设置D/C信号线
void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
	int dc = (int)t->user;
	gpio_set_level(PIN_NUM_DC, dc);
}

// 获取屏幕驱动芯片ID
uint32_t LCD_Get_ID(void)
{
	// 获取屏幕驱动芯片ID指令 0x04
	LCD_WriteCMD(0x04);
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));
	t.length=8*3;
	t.flags = SPI_TRANS_USE_RXDATA;
	t.user = (void*)1;
	esp_err_t ret = spi_device_polling_transmit(LCD_SPI_Handle, &t);
	assert( ret == ESP_OK );
	return *(uint32_t*)t.rx_data;
}
// 设置屏幕方向
void LCD_Set_Orientation(uint8_t orientation)
{
	const char *orientation_str[] = {"PORTRAIT", "PORTRAIT_INVERTED", "LANDSCAPE", "LANDSCAPE_INVERTED"};
	printf("%s->Display orientation: %s\n",TAG, orientation_str[orientation]);
	uint8_t data[] = {(1<<3)|(0<<6)|(0<<7), (1<<3)|(1<<6)|(1<<7), (1<<3)|(0<<7)|(1<<6)|(1<<5), (1<<3)|(1<<7)|(1<<5)};
	//(1<<3)|(0<<6)|(0<<7));		// BGR==1,MY==0,MX==0,MV==0
	//(1<<3)|(0<<7)|(1<<6)|(1<<5));	// BGR==1,MY==1,MX==0,MV==1
	//(1<<3)|(1<<6)|(1<<7));		// BGR==1,MY==0,MX==0,MV==0
	//(1<<3)|(1<<7)|(1<<5));		// BGR==1,MY==1,MX==0,MV==1
	LCD_Orientation = orientation;
	if(orientation == LCD_DISPLAY_ORIENTATION_PORTRAIT || orientation == LCD_DISPLAY_ORIENTATION_PORTRAIT_INVERTED){
		LCD_Width = 240;		// LCD 宽度
		LCD_Height = 320;		// LCD 高度
	}else if(orientation == LCD_DISPLAY_ORIENTATION_LANDSCAPE || orientation == LCD_DISPLAY_ORIENTATION_LANDSCAPE_INVERTED){
		LCD_Width = 320;		// LCD 宽度
		LCD_Height = 240;		// LCD 高度
	}

	printf("%s->0x36 command value: 0x%02X\n",TAG, data[orientation]);
	LCD_WriteCMD(0x36);
	LCD_WriteDate((void *) &data[orientation], 1);
}
// 清屏指定颜色
void LCD_Clear(uint16_t Color)
{
	unsigned int i,m;  
	uint8_t databuf[2] = {0,0};
	LCD_SetWindows(0,0,LCD_Width-1,LCD_Height-1);   
	for(i=0;i<LCD_Height;i++){
		for(m=0;m<LCD_Width;m++){	
			databuf[0] = (Color>>8)&0xFF;
			databuf[1] = Color&0xFF;
			LCD_WriteDate(databuf,2);
		}
	}
} 
// 设置窗口
void LCD_SetWindows(uint16_t xStar, uint16_t yStar,uint16_t xEnd,uint16_t yEnd)
{	
	uint8_t databuf[4] = {0,0,0,0};
	databuf[0] = xStar>>8;
	databuf[1] = 0xFF&xStar;	
	databuf[2] = xEnd>>8;
	databuf[3] = 0xFF&xEnd;
	LCD_WriteCMD(0x2A);
	LCD_WriteDate(databuf,4);

	databuf[0] = yStar>>8;
	databuf[1] = 0xFF&yStar;	
	databuf[2] = yEnd>>8;
	databuf[3] = 0xFF&yEnd;
	LCD_WriteCMD(0x2B);	
	LCD_WriteDate(databuf,4);

	LCD_WriteCMD(0x2C);	//开始写入GRAM			
}   

// 设置光标位置
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
	LCD_SetWindows(Xpos,Ypos,Xpos,Ypos);	
} 


/* To send a set of lines we have to send a command, 2 data bytes, another command, 2 more data bytes and another command
 * before sending the line data itself; a total of 6 transactions. (We can't put all of this in just one transaction
 * because the D/C line needs to be toggled in the middle.)
 * This routine queues these commands up as interrupt transactions so they get
 * sent faster (compared to calling spi_device_transmit several times), and at
 * the mean while the lines for next transactions can get calculated.
 */
static void send_lines(spi_device_handle_t spi, int ypos, uint16_t *linedata)
{
	esp_err_t ret;
	int x;
	//Transaction descriptors. Declared static so they're not allocated on the stack; we need this memory even when this
	//function is finished because the SPI driver needs access to it even while we're already calculating the next line.
	static spi_transaction_t trans[6];

	//In theory, it's better to initialize trans and data only once and hang on to the initialized
	//variables. We allocate them on the stack, so we need to re-init them each call.
	for (x=0; x<6; x++) {
		memset(&trans[x], 0, sizeof(spi_transaction_t));
		if ((x&1)==0) {
			//Even transfers are commands
			trans[x].length=8;
			trans[x].user=(void*)0;
		} else {
			//Odd transfers are data
			trans[x].length=8*4;
			trans[x].user=(void*)1;
		}
		trans[x].flags=SPI_TRANS_USE_TXDATA;
	}
	trans[0].tx_data[0]=0x2A;           //Column Address Set
	trans[1].tx_data[0]=0;              //Start Col High
	trans[1].tx_data[1]=0;              //Start Col Low
	trans[1].tx_data[2]=(320)>>8;       //End Col High
	trans[1].tx_data[3]=(320)&0xff;     //End Col Low
	trans[2].tx_data[0]=0x2B;           //Page address set
	trans[3].tx_data[0]=ypos>>8;        //Start page high
	trans[3].tx_data[1]=ypos&0xff;      //start page low
	trans[3].tx_data[2]=(ypos+PARALLEL_LINES)>>8;    //end page high
	trans[3].tx_data[3]=(ypos+PARALLEL_LINES)&0xff;  //end page low
	trans[4].tx_data[0]=0x2C;           //memory write
	trans[5].tx_buffer=linedata;        //finally send the line data
	trans[5].length=320*2*8*PARALLEL_LINES;          //Data length, in bits
	trans[5].flags=0; //undo SPI_TRANS_USE_TXDATA flag

	//Queue all transactions.
	for (x=0; x<6; x++) {
		ret = spi_device_queue_trans(spi, &trans[x], portMAX_DELAY);
		assert(ret==ESP_OK);
	}

	//When we are here, the SPI driver is busy (in the background) getting the transactions sent. That happens
	//mostly using DMA, so the CPU doesn't have much to do here. We're not going to wait for the transaction to
	//finish because we may as well spend the time calculating the next line. When that is done, we can call
	//send_line_finish, which will wait for the transfers to be done and check their status.
}


static void send_line_finish(spi_device_handle_t spi)
{
	spi_transaction_t *rtrans;
	esp_err_t ret;
	//Wait for all 6 transactions to be done and get back the results.
	for (int x=0; x<6; x++) {
		ret=spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
		assert(ret==ESP_OK);
		//We could inspect rtrans now if we received any info back. The LCD is treated as write-only, though.
	}
}
