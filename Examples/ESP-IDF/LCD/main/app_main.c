/* SPI Master example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "lcd.h"
#include "gui.h"
#include "pic.h"


void app_main(void)
{
	esp_err_t ret;
	Init_LCD();


	// 实心矩形
	LCD_DrawFillRectangle(20,20,100,100,GREEN);
	// 空心矩形
	LCD_DrawRectangle(20,220,100,200,GREEN);	
	// 画圆
	LCD_Draw_Circle(180,200,40,WHITE);
	// 画实心圆
	LCD_Draw_FillCircle(180,200,30,WHITE);	
	// 画点
	LCD_DrawPoint(120,110,RED);
	// 画线
	LCD_DrawLine(0, 0, 128, 128,RED);
	// 画角度线
	LCD_Draw_AngleLine(100,100,35,85,BLACK);
	// 画粗线1
	LCD_DrawBLine0(20,160,60,190,5,YELLOW);
	// 画粗线2
	LCD_DrawBLine1(40,140,90,190,2,YELLOW);
	// 画三角形
	LCD_DrawTriangel(100,50,30,100,150,150,RED);
	// 画实心三角形
	LCD_DrawFillTriangel(180,30,160,80,200,120,RED);
	// 显示单个字符
	LCD_ShowChar(120,0,BLACK,RED, 'A',12,1);
	LCD_ShowChar(140,0,BLACK,RED, 'B',12,0);	
	LCD_ShowChar(160,0,BLACK,RED, 'A',16,1);
	LCD_ShowChar(180,0,BLACK,RED, 'B',16,0);
	// 显示字符串
	LCD_ShowString(10,240,GREEN,RED,12,"ABCDabcd123",1);
	LCD_ShowString(10,252,GREEN,RED,12,"ABCDabcd123",0);
	LCD_ShowString(10,264,GREEN,RED,16,"ABCDabcd123",1);
	LCD_ShowString(10,280,GREEN,RED,16,"ABCDabcd123",0);
	// 显示数字
	LCD_ShowNum(10,296,WHITE,BLACK,123456,7,16);
	// 显示图片
	LCD_Drawbmp16(100,240,gImage_qq);
}
