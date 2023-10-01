#ifndef __GUI_H__
#define __GUI_H__
#include "stdint.h"
#include "stdbool.h"


// 画点
void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color);
// 画圆
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint16_t r,uint16_t color);
// 画实心圆
void LCD_Draw_FillCircle(uint16_t x0,uint16_t y0,uint16_t r,uint16_t color);
// 画线(单像素)
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color);
// 画角度线
void LCD_Draw_AngleLine(uint16_t x,uint16_t y,uint16_t Angle,uint16_t r,uint16_t color);
//画一条粗线(方法0)
void LCD_DrawBLine0(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint8_t size,uint16_t color);
//画一条粗线(方法1)
//x0,y0:起点 x1,y1:终点
//size:线粗细,仅支持:0~2.
//color:颜色
void LCD_DrawBLine1(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint8_t size,uint16_t color);
// 画三角形
void LCD_DrawTriangel(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color);
// 画实心三角形
void LCD_DrawFillTriangel(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color);
// 画矩形
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color);
// 实心矩形
void LCD_DrawFillRectangle(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color);
// 显示单个字符
void LCD_ShowChar(uint16_t x,uint16_t y,uint16_t bcolor,uint16_t fcolor, uint8_t ch,uint8_t size,uint8_t mode);
// 显示字符串
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t bcolor,uint16_t fcolor,uint8_t size,char *p,uint8_t mode);
// 显示数字
void LCD_ShowNum(uint16_t x,uint16_t y,uint16_t bcolor,uint16_t fcolor,uint32_t num,uint8_t len,uint8_t size);
//显示40*40 QQ图片
void LCD_Drawbmp16(uint16_t x,uint16_t y,const unsigned char *p);

uint32_t LCD_pow(uint8_t m,uint8_t n);
void LCD_Swap(uint16_t *a, uint16_t *b);
#endif
