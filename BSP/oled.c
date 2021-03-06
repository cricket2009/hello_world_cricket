/*----------------------------------------------------------------
               硬件spi
						RES						PA3
            CS	   				PA4
						D0(CLK)	  	  PA5
            DC	   				PA6
            D1(MOSI)	   	PA7

OLED的显存格式：
[0]0 1 2 3 ... 127	
[1]0 1 2 3 ... 127	
[2]0 1 2 3 ... 127	
[3]0 1 2 3 ... 127	
[4]0 1 2 3 ... 127	
[5]0 1 2 3 ... 127	
[6]0 1 2 3 ... 127	
[7]0 1 2 3 ... 127 							
	----------------------------------------------------------------*/
#include "oled.h"
#include "oledfont.h"
#include "string.h"

//write command	to	SSD1306
void oled_writeCmd(uint8_t command)
{
	
	OLED_DC_CMD; //发送命令
	SPI_Write(SPI1,command);
}

//write data	to	SSD1306
void oled_writeData(uint8_t data)
{
	
	OLED_DC_DAT; //发送数据
	SPI_Write(SPI1,data);
}

//set	Cursor
void oled_setCursor(uint8_t x, uint8_t y) 
{ 		
	
	oled_writeCmd(0xb0 + y);									//set page address
	oled_writeCmd(((x & 0xf0) >> 4) | 0x10);	//set higher column address
	oled_writeCmd((x & 0x0f));								//set lower column address
}

// clear everything
void oled_clearDisplay(void)
{
	
	uint8_t i, n;
	
	OLED_CS_L;
	
	for(i=0;i < 8;i++)  
	{  		
		oled_setCursor(0, i);
		
		for(n=0; n < 128; n++)
		{
			oled_writeData(0x00);
		}
	}
	
	OLED_CS_H;
}

// clear 4 line
void oled_clearLine(char* line1, char* line2, char* line3, char* line4)  
{  
	uint8_t i = 0;
	int8_t N1, N2, N3, N4; //这一次的字符串长度
	static int8_t L1 = 0, L2 = 0, L3 = 0, L4 = 0; //上一次的字符串长度
	int8_t	D1, D2, D3, D4; //字符串长度差值
	
	N1 = strlen(line1);
	N2 = strlen(line2);
	N3 = strlen(line3);
	N4 = strlen(line4);
	
	//计算要清0的字符数
	D1 = L1 - N1;
	D2 = L2 - N2;
	D3 = L3 - N3;
	D4 = L4 - N4;
	
	//保存这次的字符长度作为上一次的字符长度
	L1 = N1;
	L2 = N2;
	L3 = N3;
	L4 = N4;
	
	if(D1 > 0 || D2 > 0 || D3 > 0 || D4 > 0)
	{		
		OLED_CS_L;
		
		if(D1 > 0)	//如果上一次字符长度比这次的长，则清除多余的字符，否则不清屏
		{		
			for(i=0;i < 2;i++)
			{
				oled_setCursor((L1 << 3), i);	//设置起始清除地址oled_setCursor((line1_last * 8), i);
				
				uint16_t n;
				for(n=0; n < (D1 << 3) ;n++)
				{
					oled_writeData(0x00);	//写0清除数据
				}
			}
		}
		if(D2 > 0)
		{
			for(i=2;i < 4;i++)
			{
				oled_setCursor((L2 << 3), i);
				
				uint16_t n;
				for(n=0; n < (D2 << 3) ;n++)
				{
					oled_writeData(0x00);	//写0清除数据
				}
			}
		}
		if(D3 > 0)
		{			
			for(i=4;i < 6;i++)
			{
				oled_setCursor((L3 << 3), i);
				
				uint16_t n;
				for(n=0; n < (D3 << 3) ;n++)
				{
					oled_writeData(0x00);	//写0清除数据
				}
			}
		}
		if(D4 > 0)
		{
			for(i=6;i < 8;i++)
			{
				oled_setCursor((L4 << 3), i);
				
				uint16_t n;
				for(n=0; n < (D4 << 3) ;n++)
				{
					oled_writeData(0x00);	//写0清除数据
				}
			}
		}
		
		OLED_CS_H;
	}
}

//draw a char
void oled_drawChar(uint8_t x, uint8_t y, char c)
{      		
	uint16_t i;
	
	uint32_t c_offset = c - ' ';
	
	if(x > 120 || y > 8)
		return;

	oled_setCursor(x,y);
	for(i=0; i < F_heigth; i++)
	{
		oled_writeData(F8X16[(c_offset << 4) + i]); //F8X16[c_offset * F_size + i]
	}

	oled_setCursor(x,y+1);
	for(i=0; i < F_heigth; i++)
	{
		oled_writeData(F8X16[(c_offset << 4) + i + 8]); //F8X16[c_offset * F_size + i + 8]
	}
}

void oled_drawStr(uint8_t x, uint8_t y, char* s)
{
	uint8_t j = 0;
	
	OLED_CS_L;	
	
	while (s[j]	!=	'\0')
	{		
		oled_drawChar(x,y,s[j]);
		x += F_heigth;
		j++;
	}
	
	OLED_CS_H;
}

void oled_begin(void)
{				
	OLED_CS_L;
	
	oled_writeCmd(0xae);//--turn off oled panel
	oled_writeCmd(0x00);//---set low column address
	oled_writeCmd(0x10);//---set high column address
	oled_writeCmd(0x40);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	oled_writeCmd(0x81);//--set contrast control register
	oled_writeCmd(0xcf); // Set SEG Output Current Brightness
	oled_writeCmd(0xa1);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
	oled_writeCmd(0xc8);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
	oled_writeCmd(0xa6);//--set normal display
	oled_writeCmd(0xa8);//--set multiplex ratio(1 to 64)
	oled_writeCmd(0x3f);//--1/64 duty
	oled_writeCmd(0xd3);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	oled_writeCmd(0x00);//-not offset
	oled_writeCmd(0xd5);//--set display clock divide ratio/oscillator frequency
	oled_writeCmd(0x80);//--set divide ratio, Set Clock as 100 Frames/Sec
	oled_writeCmd(0xd9);//--set pre-charge period
	oled_writeCmd(0xf1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	oled_writeCmd(0xda);//--set com pins hardware configuration
	oled_writeCmd(0x12);
	oled_writeCmd(0xdb);//--set vcomh
	oled_writeCmd(0x40);//Set VCOM Deselect Level
	oled_writeCmd(0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)
	oled_writeCmd(0x02);//
	oled_writeCmd(0x8d);//--set Charge Pump enable/disable
	oled_writeCmd(0x14);//--set(0x10) disable
	oled_writeCmd(0xa4);// Disable Entire Display On (0xa4/0xa5)
	oled_writeCmd(0xa6);// Disable Inverse Display On (0xa6/a7) 
	oled_writeCmd(0xaf);//--turn on oled panel

	OLED_CS_H;

	oled_clearDisplay();
}
