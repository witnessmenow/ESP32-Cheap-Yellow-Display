// XPT2046.c


/*********************
 *      INCLUDES
 *********************/
#include "xpt2046.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include <stddef.h>
#include "../LCD/include/lcd.h"
#include "../GUI/include/gui.h"
#include "math.h"  


/*********************
 *      DEFINES
 *********************/
#define TAG "XPT2046"

#define CMD_X_READ  0b10010000      //0x90
#define CMD_Y_READ  0b11010000      //0xD0
#define Read_Count		30			// 读取次数

// 校正时采用横屏计算
#define TP_Adjust_Width			320
#define TP_Adjust_Height		240

float xfac;
float yfac;
short xoff;
short yoff;
uint16_t TouchX = 0;
uint16_t TouchY = 0;

//初始化 XPT2046
void xpt2046_init(void)
{
    gpio_config_t irq_config = {
        .pin_bit_mask = BIT64(XPT2046_IRQ),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t ret = gpio_config(&irq_config);
    /**/
    gpio_config_t miso_config = {
        .pin_bit_mask = BIT64(XPT2046_MISO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ret = gpio_config(&miso_config);
    esp_rom_gpio_pad_select_gpio(XPT2046_MOSI);
    gpio_set_direction(XPT2046_MOSI, GPIO_MODE_OUTPUT);// 设置GPIO为推挽输出模式
    esp_rom_gpio_pad_select_gpio(XPT2046_CLK);
    gpio_set_direction(XPT2046_CLK, GPIO_MODE_OUTPUT);// 设置GPIO为推挽输出模式
    esp_rom_gpio_pad_select_gpio(XPT2046_CS);
    gpio_set_direction(XPT2046_CS, GPIO_MODE_OUTPUT);// 设置GPIO为推挽输出模式
    
    printf("%s->XPT2046 Initialization\n",TAG);
    assert(ret == ESP_OK);
}

/**
 * Get the current position and state of the touchpad
 * @param data store the read data here
 * @return false: because no more data to be read
 */
bool xpt2046_read(void)
{
	static int16_t last_x = 0,last_y = 0;
	bool valid = true;
	int16_t swap_tmp;
	uint16_t x = 0,y = 0;
	uint16_t ux = 0,uy = 0;
	uint8_t irq = gpio_get_level(XPT2046_IRQ);
	if (irq == 0) {
		ux = TP_Read_XOY(CMD_X_READ);   //5700   61700
		uy = TP_Read_XOY(CMD_Y_READ);   //3000   62300

		//printf("%s->XPT2046 Read ADC(X:%d,Y:%d)\n",TAG, ux, uy);

		x = xfac * ux + xoff;//将结果转换为屏幕坐标
		y = yfac * uy + yoff; 
/*
校准后采集的触摸坐标系，要结合当前液晶显示坐标系翻转、调换XY，将触摸坐标系转为显示坐标系
				X0						X320
			Y0	-----------------------------
				|							|
				|							|
				软							|
				排							|
				线							|
				|							|
				|							|
			Y240-----------------------------
*/
		//printf("%s->LCD_DISPLAY_ORIENTATION_PORTRAIT  %d\n",TAG,LCD_Orientation);
		if(LCD_Orientation == LCD_DISPLAY_ORIENTATION_PORTRAIT){// 纵向
			// 调换XY
			swap_tmp = x;
			x = y;
			y = swap_tmp;
			// 翻转Y
			y =  LCD_Height - y;
		}else if(LCD_Orientation == LCD_DISPLAY_ORIENTATION_PORTRAIT_INVERTED){// 纵向翻转
			// 调换XY
			swap_tmp = x;
			x = y;
			y = swap_tmp;
			// 翻转X
			x =  LCD_Width - x;
		}else if(LCD_Orientation == LCD_DISPLAY_ORIENTATION_LANDSCAPE){// 横向
			// 翻转X
			x =  LCD_Width - x;
			// 翻转Y
			y =  LCD_Height - y;			
		}else if(LCD_Orientation == LCD_DISPLAY_ORIENTATION_LANDSCAPE_INVERTED){// 横向翻转

		}
		//printf("%s->XPT2046 Read Touch(X:%d,Y:%d)\n",TAG, x, y);
		TouchX = x;
		TouchY = y;
		return true;
	}
	return false;
}
uint16_t TP_Read_XOY(uint8_t xy)
{
	uint8_t LOST_VAL = 1;//丢弃值  
	uint16_t i,j,temp,buf[Read_Count];
	uint32_t sum=0;
	for(i=0;i<Read_Count;i++){
		buf[i]=xpt2046_gpio_spi_read_reg(xy);
	}
	for(i=0;i<Read_Count-1; i++){//排序
		for(j=i+1;j<Read_Count;j++){
			if(buf[i]>buf[j]){//升序排列
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}
	sum=0;
	for(i=LOST_VAL;i<Read_Count-LOST_VAL;i++){
		sum+=buf[i];
	}
	temp = sum/(Read_Count-2*LOST_VAL);
	return temp;
}
void xpt2046_gpio_Write_Byte(uint8_t num)    
{  
	uint8_t count=0;   
	for(count=0;count<8;count++){
		if(num&0x80){
			gpio_set_level(XPT2046_MOSI, 1);  
		}else{
			gpio_set_level(XPT2046_MOSI, 0);
		}  
		num<<=1; 
		gpio_set_level(XPT2046_CLK, 0);
		gpio_set_level(XPT2046_CLK, 1);
	}
}
uint16_t xpt2046_gpio_spi_read_reg(uint8_t reg)
{
	uint8_t count=0;
	uint16_t ADValue=0;
	gpio_set_level(XPT2046_CLK, 0);		// 先拉低时钟 	 
	gpio_set_level(XPT2046_MOSI, 0); 	// 拉低数据线
	gpio_set_level(XPT2046_CS, 0); 		// 选中触摸屏IC
	xpt2046_gpio_Write_Byte(reg);		// 发送命令字
	esp_rom_delay_us(6);					// ADS7846的转换时间最长为6us
	gpio_set_level(XPT2046_CLK, 0);
	esp_rom_delay_us(1);
	gpio_set_level(XPT2046_CLK, 1);		// 给1个时钟，清除BUSY
	gpio_set_level(XPT2046_CLK, 0);
	for(count=0;count<16;count++){		// 读出16位数据,只有高12位有效
		ADValue<<=1; 	 
		gpio_set_level(XPT2046_CLK, 0);	// 下降沿有效
		gpio_set_level(XPT2046_CLK, 1);	
		if(gpio_get_level(XPT2046_MISO))ADValue++;
	}  	
	ADValue>>=4;						// 只有高12位有效.
	gpio_set_level(XPT2046_CS, 1);		// 释放片选
	return(ADValue);
}
void TP_Adjust(void)
{
	uint8_t Last_LCD_Orientation = LCD_Orientation;// 记录校正前屏幕方向，完成后还原
	uint16_t pos_temp[4][2];//坐标缓存值
	uint8_t  cnt=0,GetTouchOK = 0;	
	uint16_t d1,d2;
	uint32_t tem1,tem2;
	float fac; 	
	uint16_t outtime=0;
 	cnt=0;
	uint16_t rX,rY;
	// 校正触摸时要显示为纵向
	LCD_Set_Orientation(LCD_DISPLAY_ORIENTATION_PORTRAIT);// 纵向
	LCD_Clear(WHITE);//清屏
	LCD_ShowString(10,40,WHITE,BLUE,16,"Please use the stylus click",1);//显示提示信息
	LCD_ShowString(10,56,WHITE,BLUE,16,"the cross on the screen.",1);//显示提示信息
	LCD_ShowString(10,72,WHITE,BLUE,16,"The cross will always move",1);//显示提示信息
	LCD_ShowString(10,88,WHITE,BLUE,16,"until the screen adjustment",1);//显示提示信息
	LCD_ShowString(10,104,WHITE,BLUE,16,"is completed.",1);//显示提示信息
	TP_Drow_Touch_Point(20,20,RED);//画点1 
	while(1){//如果连续10秒钟没有按下,则自动退出
		vTaskDelay(10 / portTICK_PERIOD_MS);
		if (gpio_get_level(XPT2046_IRQ) == 0) {
			rX = TP_Read_XOY(CMD_X_READ);
			rY = TP_Read_XOY(CMD_Y_READ);
			while(!gpio_get_level(XPT2046_IRQ)){
				vTaskDelay(100 / portTICK_PERIOD_MS);
			}
			GetTouchOK = 1;
		}
		if(GetTouchOK==1){
			GetTouchOK = 0;
			pos_temp[cnt][0]=rX;
			pos_temp[cnt][1]=rY;
			cnt++;
			switch(cnt){
				case 1:
					TP_Drow_Touch_Point(20,20,WHITE);						// 清除点1 
					TP_Drow_Touch_Point(LCD_Width-20,20,RED);				// 画点2
					vTaskDelay(1000 / portTICK_PERIOD_MS);
					break;
				case 2:
					TP_Drow_Touch_Point(LCD_Width-20,20,WHITE);				// 清除点2
					TP_Drow_Touch_Point(20,LCD_Height-20,RED);				// 画点3
					vTaskDelay(1000 / portTICK_PERIOD_MS);
					break;
				case 3:
					TP_Drow_Touch_Point(20,LCD_Height-20,WHITE);			// 清除点3
					TP_Drow_Touch_Point(LCD_Width-20,LCD_Height-20,RED);	// 画点4
					break;
				case 4:	 //全部四个点已经得到
					//对边相等
					tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2
					tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//得到1,2的距离
					
					tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
					tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//得到3,4的距离
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05||d1==0||d2==0){//不合格{
						cnt=0;
						TP_Drow_Touch_Point(LCD_Width-20,LCD_Height-20,WHITE);	//清除点4
						TP_Drow_Touch_Point(20,20,RED);								//画点1
						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据   
						continue;
					}
					tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//得到1,3的距离
					
					tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//得到2,4的距离
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05){//不合格{
						cnt=0;
						TP_Drow_Touch_Point(LCD_Width-20,LCD_Height-20,WHITE);	//清除点4
						TP_Drow_Touch_Point(20,20,RED);								//画点1
						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据   
						continue;
					}//正确了
									
					//对角线相等
					tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3
					tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3
					tem1*=tem1;
					tem2*=tem2;
					d1=sqrt(tem1+tem2);//得到1,4的距离

					tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4
					tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4
					tem1*=tem1;
					tem2*=tem2;
					d2=sqrt(tem1+tem2);//得到2,3的距离
					fac=(float)d1/d2;
					if(fac<0.95||fac>1.05)//不合格
					{
						cnt=0;
						TP_Drow_Touch_Point(LCD_Width-20,LCD_Height-20,WHITE);	//清除点4
						TP_Drow_Touch_Point(20,20,RED);								//画点1
						TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据   
						continue;
					}//正确了
					//计算结果
					xfac=(float)(TP_Adjust_Width-40)/(pos_temp[0][0] - pos_temp[2][0]);	// 得到xfac	点2-点1的X AD差值
					xoff=(TP_Adjust_Width-xfac*(pos_temp[0][0]+pos_temp[2][0]))/2;		// 得到xoff		
					yfac=(float)(TP_Adjust_Height-40)/(pos_temp[3][1] - pos_temp[2][1]);// 得到yfac	点4-点3的Y AD差值
					yoff=(TP_Adjust_Height-yfac*(pos_temp[3][1]+pos_temp[2][1]))/2;		// 得到yoff

					printf("%s->XPT2046 Read fac(%f,%f)\n",TAG, xfac, yfac);
					printf("%s->XPT2046 Read off(%d,%d)\n",TAG, xoff, yoff);
					printf("%s->XPT2046 Read pos_temp1(X %d,Y %d)\n",TAG, pos_temp[0][0], pos_temp[0][1]);
					printf("%s->XPT2046 Read pos_temp2(X %d,Y %d)\n",TAG, pos_temp[1][0], pos_temp[1][1]);
					printf("%s->XPT2046 Read pos_temp3(X %d,Y %d)\n",TAG, pos_temp[2][0], pos_temp[2][1]);
					printf("%s->XPT2046 Read pos_temp4(X %d,Y %d)\n",TAG, pos_temp[3][0], pos_temp[3][1]);

					if(abs(xfac)>2||abs(yfac)>2){//触屏和预设的相反了.
						cnt=0;
						TP_Drow_Touch_Point(LCD_Width-20,LCD_Height-20,WHITE);	//清除点4
						TP_Drow_Touch_Point(20,20,RED);								//画点1
						LCD_ShowString(40,26,WHITE,BLUE, 16,"TP Need readjust!",1);
						vTaskDelay(1000 / portTICK_PERIOD_MS);
					}		
					LCD_Clear(WHITE);//清屏
					LCD_Set_Orientation(Last_LCD_Orientation);// 校正完成后还原开始时屏幕方向
					LCD_ShowString(35,110,WHITE,BLUE, 16,"Touch Screen Adjust OK!",1);//校正完成
					vTaskDelay(1000 / portTICK_PERIOD_MS);
					//TP_Save_Adjdata();  
					//LCD_Clear(WHITE);//清屏   
					return;//校正完成
			}
		}
 	}
}
void TP_Drow_Touch_Point(uint16_t x,uint16_t y,uint16_t color)
{
	LCD_DrawLine(x-12,y,x+13,y,color);//横线
	LCD_DrawLine(x,y-12,x,y+13,color);//竖线
	LCD_DrawPoint(x+1,y+1,color);
	LCD_DrawPoint(x-1,y+1,color);
	LCD_DrawPoint(x+1,y-1,color);
	LCD_DrawPoint(x-1,y-1,color);
	LCD_Draw_Circle(x,y,6,color);//画中心圈
}	

void TP_Adj_Info_Show(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t x3,uint16_t y3,uint16_t fac)
{	  
	LCD_ShowString(40,140,WHITE,BLACK,16,"x1:",1);
 	LCD_ShowString(40+80,140,WHITE,BLACK,16,"y1:",1);
 	LCD_ShowString(40,160,WHITE,BLACK,16,"x2:",1);
 	LCD_ShowString(40+80,160,WHITE,BLACK, 16,"y2:",1);
	LCD_ShowString(40,180,WHITE,BLACK, 16,"x3:",1);
 	LCD_ShowString(40+80,180,WHITE,BLACK, 16,"y3:",1);
	LCD_ShowString(40,200,WHITE,BLACK, 16,"x4:",1);
 	LCD_ShowString(40+80,200,WHITE,BLACK, 16,"y4:",1);  
 	LCD_ShowString(40,220,WHITE,BLACK, 16,"fac is:",1);     
	LCD_ShowNum(40+24,140,WHITE,BLACK,x0,4,16);		//显示数值
	LCD_ShowNum(40+24+80,140,WHITE,BLACK,y0,4,16);	//显示数值
	LCD_ShowNum(40+24,160,WHITE,BLACK,x1,4,16);		//显示数值
	LCD_ShowNum(40+24+80,160,WHITE,BLACK,y1,4,16);	//显示数值
	LCD_ShowNum(40+24,180,WHITE,BLACK,x2,4,16);		//显示数值
	LCD_ShowNum(40+24+80,180,WHITE,BLACK,y2,4,16);	//显示数值
	LCD_ShowNum(40+24,200,WHITE,BLACK,x3,4,16);		//显示数值
	LCD_ShowNum(40+24+80,200,WHITE,BLACK,y3,4,16);	//显示数值
 	LCD_ShowNum(40+56,220,WHITE,BLACK,fac,3,16); 	//显示数值,该数值必须在95~105范围之内.
}
void TP_Save_Adjdata(void)
{
/*	int32_t temp;			 
	//保存校正结果!		   							  
	temp=tp_dev.xfac*100000000;//保存x校正因素      
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE,temp,4);   
	temp=tp_dev.yfac*100000000;//保存y校正因素    
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE+4,temp,4);
	//保存x偏移量
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE+8,tp_dev.xoff,2);		    
	//保存y偏移量
	AT24CXX_WriteLenByte(SAVE_ADDR_BASE+10,tp_dev.yoff,2);	
	//保存触屏类型
	AT24CXX_WriteOneByte(SAVE_ADDR_BASE+12,tp_dev.touchtype);	
	temp=0X0A;//标记校准过了
	AT24CXX_WriteOneByte(SAVE_ADDR_BASE+13,temp); 
	*/
}


