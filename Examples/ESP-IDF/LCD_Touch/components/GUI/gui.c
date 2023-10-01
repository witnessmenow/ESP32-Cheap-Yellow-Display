#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stdint.h"
#include "stdbool.h"
#include "lcd.h"
#include "gui.h"
#include "font.h"
#include "math.h"  
#include "string.h"
//#include "pic.h"

//π值定义
#define	app_pi	3.1415926535897932384626433832795 

// 画点
void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
	LCD_SetCursor(x,y);//设置光标位置 
	LCD_WriteDate16(color); 
}
// 画点1
void LCD_DrawPoint1(uint16_t x,uint16_t y,uint16_t color)
{
	LCD_DrawRectangle(x,y,x+1,y+1,color);
}
// 画圆
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint16_t r,uint16_t color)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //判断下个点位置的标志
	while(a<=b){
		LCD_DrawPoint(x0+a,y0-b,color);		//5
 		LCD_DrawPoint(x0+b,y0-a,color);		//0
		LCD_DrawPoint(x0+b,y0+a,color);		//4
		LCD_DrawPoint(x0+a,y0+b,color);		//6
		LCD_DrawPoint(x0-a,y0+b,color);		//1
 		LCD_DrawPoint(x0-b,y0+a,color);
		LCD_DrawPoint(x0-a,y0-b,color);		//2
  		LCD_DrawPoint(x0-b,y0-a,color);		//7
		a++;
		//使用Bresenham算法画圆
		if(di<0){
			di +=4*a+6;
		}else{
			di+=10+4*(a-b);
			b--;
		}
	}
}
// 画实心圆
void LCD_Draw_FillCircle(uint16_t x0,uint16_t y0,uint16_t r,uint16_t color)
{
	uint16_t a,b;
	int di;//uint16_t di 画实心菱形
	a=0;b=r;
	di=3-(r<<1);	// 判断下个点位置的标志
	while(a<=b){
		int i = a,p = b;
		while(i>0){
			LCD_DrawPoint(x0+b,y0-i,color);
			LCD_DrawPoint(x0-i,y0+b,color);
			i--;
		}
		while(p>0){
			LCD_DrawPoint(x0-a,y0-p,color);
			LCD_DrawPoint(x0-p,y0-a,color);
			LCD_DrawPoint(x0+a,y0-p,color);
			LCD_DrawPoint(x0-p,y0+a,color);
			LCD_DrawPoint(x0+a,y0+p,color);
			LCD_DrawPoint(x0+p,y0+a,color);
			p--;
		}
		a++;
		//Bresenham算法画圆
		if(di<0){
			di +=4*a+6;
		}else{
			di+=10+4*(a-b);
			b--;
		}
	}
	LCD_DrawPoint(x0,y0,color); //圆心坐标
}



// 画线(单像素)
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color)
{
	uint16_t t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ ){//画线输出
		LCD_DrawPoint(uRow,uCol,color);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) { 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) { 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}
// 画角度线
void LCD_Draw_AngleLine(uint16_t x,uint16_t y,uint16_t Angle,uint16_t r,uint16_t color)
{
	int sx=x-r;
	int sy=y-r;
	int px0,px1;
	int py0,py1;  
	uint8_t r1; 
	int d = r;
	r1=d/2+3;
	px0=x;//sx+r+(r-d-7)*sin((app_pi/30)*Angle); 
	py0=y;//sy+r-(r-d-7)*cos((app_pi/30)*Angle); 
	px1=sx+r+r1*sin((app_pi/180)*Angle); 
	py1=sy+r-r1*cos((app_pi/180)*Angle); 
	LCD_DrawLine(px0,py0,px1,py1,color);
}
//画一条粗线(方法0)
//(x1,y1),(x2,y2):线条的起止坐标
//size：线条的粗细程度
void LCD_DrawBLine0(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint8_t size,uint16_t color)
{
	uint16_t t; 
	uint16_t xerr=0,yerr=0,delta_x,delta_y,distance; 
	uint16_t incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return;
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		LCD_Draw_FillCircle(uRow,uCol,size,color);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance){xerr-=distance;uRow+=incx;}  
		if(yerr>distance){yerr-=distance;uCol+=incy;}
	}  
} 

//画一条粗线(方法1)
//x0,y0:起点 x1,y1:终点
//size:线粗细,仅支持:0~2.
//color:颜色
void LCD_DrawBLine1(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint8_t size,uint16_t color)
{
	int t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 

	delta_x=x1-x0; //计算坐标增量 
	delta_y=y1-y0; 
	uRow=x0; 
	uCol=y0; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ ){//画线输出 
		if(size==0)LCD_DrawPoint(uRow,uCol,color);//画点 
		if(size==1){
			LCD_DrawPoint(uRow,uCol,color);//画点 
			LCD_DrawPoint(uRow+1,uCol,color);//画点 
			LCD_DrawPoint(uRow,uCol+1,color);//画点 
			LCD_DrawPoint(uRow+1,uCol+1,color);//画点 
		}
		if(size==2){
			LCD_DrawPoint(uRow,uCol,color);//画点 
			LCD_DrawPoint(uRow+1,uCol,color);//画点 
			LCD_DrawPoint(uRow,uCol+1,color);//画点 
			LCD_DrawPoint(uRow+1,uCol+1,color);//画点 
			LCD_DrawPoint(uRow-1,uCol+1,color);//画点 
			LCD_DrawPoint(uRow+1,uCol-1,color);//画点
			LCD_DrawPoint(uRow-1,uCol-1,color);//画点  
			LCD_DrawPoint(uRow-1,uCol,color);//画点 
			LCD_DrawPoint(uRow,uCol-1,color);//画点  
		}
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) { 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) { 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}
}
// 画三角形
void LCD_DrawTriangel(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color)
{
	LCD_DrawLine(x0,y0,x1,y1,color);
	LCD_DrawLine(x1,y1,x2,y2,color);
	LCD_DrawLine(x2,y2,x0,y0,color);
}

// 画实心三角形
void LCD_DrawFillTriangel(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color)
{
	uint16_t a, b, y, last;
	int dx01, dy01, dx02, dy02, dx12, dy12;
	long sa = 0;
	long sb = 0;
	if (y0 > y1) {
		LCD_Swap(&y0,&y1); 
		LCD_Swap(&x0,&x1);
	}
	if (y1 > y2) {
		LCD_Swap(&y2,&y1); 
		LCD_Swap(&x2,&x1);
	}
	if (y0 > y1) {
		LCD_Swap(&y0,&y1); 
		LCD_Swap(&x0,&x1);
	}
	if(y0 == y2) { 
		a = b = x0;
		if(x1 < a){
			a = x1;
		}else if(x1 > b){
			b = x1;
		}
		if(x2 < a){
			a = x2;
		}else if(x2 > b){
			b = x2;
		}
		LCD_DrawFillRectangle(a,y0,b,y0,color);
		return;
	}
	dx01 = x1 - x0;
	dy01 = y1 - y0;
	dx02 = x2 - x0;
	dy02 = y2 - y0;
	dx12 = x2 - x1;
	dy12 = y2 - y1;
	
	if(y1 == y2){
		last = y1; 
	}else{
		last = y1-1; 
	}
	for(y=y0; y<=last; y++) {
		a = x0 + sa / dy01;
		b = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;
		if(a > b){
			LCD_Swap(&a,&b);
		}
		LCD_DrawFillRectangle(a,y,b,y,color);
	}
	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);
	for(; y<=y2; y++) {
		a = x1 + sa / dy12;
		b = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		if(a > b){
			LCD_Swap(&a,&b);
		}
		LCD_DrawFillRectangle(a,y,b,y,color);
	}
}
// 画矩形
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color)
{
	LCD_DrawLine(x1,y1,x2,y1,color);
	LCD_DrawLine(x1,y1,x1,y2,color);
	LCD_DrawLine(x1,y2,x2,y2,color);
	LCD_DrawLine(x2,y1,x2,y2,color);
}  
// 实心矩形
void LCD_DrawFillRectangle(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color)
{
	uint16_t i,j;			
	uint16_t width=ex-sx+1; 		// 得到填充的宽度
	uint16_t height=ey-sy+1;		// 高度
	LCD_SetWindows(sx,sy,ex,ey);	// 设置显示窗口
	for(i=0;i<height;i++){
		for(j=0;j<width;j++)
		LCD_WriteDate16(color);		// 写入数据
	}
	LCD_SetWindows(0,0,LCD_Width-1,LCD_Height-1);//恢复窗口设置为全屏
}

// 显示单个字符
void LCD_ShowChar(uint16_t x,uint16_t y,uint16_t bcolor,uint16_t fcolor, uint8_t ch,uint8_t size,uint8_t mode)
{
	uint8_t temp,pos,t;
	ch=ch-' ';//得到偏移后的值
	LCD_SetWindows(x,y,x+size/2-1,y+size-1);//设置单个文字显示窗口
	if(!mode){ //非叠加方式
		for(pos=0;pos<size;pos++){
			if(size==12)temp=asc2_1206[ch][pos];//调用1206字体
			else temp=asc2_1608[ch][pos];		 //调用1608字体
			for(t=0;t<size/2;t++){
				if(temp&0x01)LCD_WriteDate16(fcolor); 
				else LCD_WriteDate16(bcolor); 
				temp>>=1; 
			}
		}
	}else{//叠加方式
		for(pos=0;pos<size;pos++){
			if(size==12)temp=asc2_1206[ch][pos];//调用1206字体
			else temp=asc2_1608[ch][pos];		 //调用1608字体
			for(t=0;t<size/2;t++){
				if(temp&0x01)LCD_DrawPoint(x+t,y+pos,fcolor);//画一个点
				temp>>=1; 
			}
		}
	}
	LCD_SetWindows(0,0,LCD_Width-1,LCD_Height-1);//恢复窗口为全屏
}
// 显示字符串
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t bcolor,uint16_t fcolor,uint8_t size,char *p,uint8_t mode)
{
	while((*p<='~')&&(*p>=' ')){//判断是不是非法字符!
		if(x>(LCD_Width-1)||y>(LCD_Height-1)) 
		return;
		LCD_ShowChar(x,y,bcolor,fcolor,*p,size,mode);
		x+=size/2;
		p++;
	}
} 
// 显示数字
void LCD_ShowNum(uint16_t x,uint16_t y,uint16_t bcolor,uint16_t fcolor,uint32_t num,uint8_t len,uint8_t size)
{
	uint8_t t,temp,enshow=0;
	for(t=0;t<len;t++){
		temp=(num/LCD_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1)){
			if(temp==0){
				LCD_ShowChar(x+(size/2)*t,y,bcolor,fcolor,' ',size,0);
				continue;
			}else enshow=1;
		}
	 	LCD_ShowChar(x+(size/2)*t,y,bcolor,fcolor,temp+'0',size,0);
	}
} 
//显示40*40 QQ图片
void LCD_Drawbmp16(uint16_t x,uint16_t y,const unsigned char *p) 
{
  	int i; 
	unsigned char picH,picL; 
	LCD_SetWindows(x,y,x+40-1,y+40-1);//窗口设置
    for(i=0;i<40*40;i++)
	{	
	 	picL=*(p+i*2);	//数据低位在前
		picH=*(p+i*2+1);				
		LCD_WriteDate16(picH<<8|picL);  						
	}	
	LCD_SetWindows(0,0,LCD_Width-1,LCD_Height-1);//恢复窗口为全屏
}















uint32_t LCD_pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}
void LCD_Swap(uint16_t *a, uint16_t *b)
{
	uint16_t tmp;
	tmp = *a;
	*a = *b;
	*b = tmp;
}
