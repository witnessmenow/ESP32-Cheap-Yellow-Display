
#ifndef XPT2046_H
#define XPT2046_H

#include <stdint.h>
#include <stdbool.h>

#define XPT2046_IRQ			36
#define XPT2046_MOSI		32
#define XPT2046_MISO		39
#define XPT2046_CLK			25
#define XPT2046_CS			33

extern uint16_t TouchX;
extern uint16_t TouchY;

void xpt2046_init(void);
uint16_t xpt2046_gpio_spi_read_reg(uint8_t reg);
void xpt2046_gpio_Write_Byte(uint8_t num);
uint16_t TP_Read_XOY(uint8_t xy);
bool xpt2046_read(void);


void TP_Adjust(void);
void TP_Drow_Touch_Point(uint16_t x,uint16_t y,uint16_t color);
void TP_Adj_Info_Show(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t x3,uint16_t y3,uint16_t fac);
#endif /* XPT2046_H */




/*
点1X坐标pos_temp[0][0]		点1Y坐标pos_temp[0][1]
点2X坐标pos_temp[1][0]		点2Y坐标pos_temp[1][1]
点3X坐标pos_temp[2][0]		点3Y坐标pos_temp[2][1]
点4X坐标pos_temp[3][0]		点4Y坐标pos_temp[3][1]

X坐标差值：pos_temp[0][0] - pos_temp[2][0]
Y坐标差值：pos_temp[3][1] - pos_temp[2][1]
	------------------------------------------------
X320|	点1								点2			|
	|	ADX:3672						ADX:3691	|
	|	ADY:513							ADY:3600	|
	|												|
	|												|
	|												|
	|												|
	|												|
	|												|
	|												|
	|												|
	|												|
	|												|
	|												|
	|												|
	|												|
	|												|
	|												|
	|												|
	|												|
	|	点3								点4			|
	|	ADX:578							ADX:540		|
	|	ADY:418							ADY:3612	|	
X0	-------------------------------------------------
	Y0											Y240
				||||||排线方向|||||||
*/