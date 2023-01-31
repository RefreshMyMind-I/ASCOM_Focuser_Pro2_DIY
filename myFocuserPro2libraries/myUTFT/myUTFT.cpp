/*
  UTFT.cpp - Arduino/chipKit library support for Color TFT LCD Boards
  Copyright (C)2010-2012 Henning Karlsen. All right reserved
  
  This library is the continuation of my ITDB02_Graph, ITDB02_Graph16
  and RGB_GLCD libraries for Arduino and chipKit. As the number of 
  supported display modules and controllers started to increase I felt 
  it was time to make a single, universal library as it will be much 
  easier to maintain in the future.

  Basic functionality of this library was origianlly based on the 
  demo-code provided by ITead studio (for the ITDB02 modules) and 
  NKC Electronics (for the RGB GLCD module/shield).

  This library supports a number of 8bit, 16bit and serial graphic 
  displays, and will work with both Arduino and chipKit boards. For a 
  full list of tested display modules and controllers, see the 
  document UTFT_Supported_display_modules_&_controllers.pdf.

  When using 8bit and 16bit display modules there are some 
  requirements you must adhere to. These requirements can be found 
  in the document UTFT_Requirements.pdf.
  There are no special requirements when using serial displays.

  You can always find the latest version of the library at 
  http://electronics.henningkarlsen.com/

  If you make any modifications or improvements to the code, I would 
  appreciate that you share the code with me so that I might include 
  it in the next release. I can be contacted through 
  http://electronics.henningkarlsen.com/contact.php.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "myUTFT.h"
#include <pins_arduino.h>
#if defined(__AVR__)
	#include <avr/pgmspace.h>
	#include "myHW_AVR.h" 
#else
	#include "myHW_PIC32.h"
#endif
#include "mymemorysaver.h"

UTFT::UTFT()
{
}
//SDA SCLK CS RST RS
UTFT::UTFT(byte model, int RS, int WR,int CS, int RST, int SER)
{ 
	switch (model)
	{
		case ILI9341://for 2.2 inch TianMa TFT Displayer
			disp_x_size=239;
			disp_y_size=319;
			display_transfer_mode=1;
			display_serial_mode=SERIAL_5PIN;
			break;
	}
	display_model=model;

	//SERIAL Mode
	{
		P_SDA	= portOutputRegister(digitalPinToPort(RS));
		B_SDA	= digitalPinToBitMask(RS);
		P_SCL	= portOutputRegister(digitalPinToPort(WR));
		B_SCL	= digitalPinToBitMask(WR);
		P_CS	= portOutputRegister(digitalPinToPort(CS));
		B_CS	= digitalPinToBitMask(CS);
		P_RST	= portOutputRegister(digitalPinToPort(RST));
		B_RST	= digitalPinToBitMask(RST);
		if (display_serial_mode!=SERIAL_4PIN)
		{
			P_RS	= portOutputRegister(digitalPinToPort(SER));
			B_RS	= digitalPinToBitMask(SER);
			pinMode(SER,OUTPUT);
		}
		pinMode(RS,OUTPUT);
		pinMode(WR,OUTPUT);
		pinMode(CS,OUTPUT);
		pinMode(RST,OUTPUT);
	}
}

void UTFT::LCD_Write_COM(char VL)  
{   
	LCD_Writ_Bus(0x00,VL,display_transfer_mode);
}

void UTFT::LCD_Write_DATA(char VH,char VL)
{
	LCD_Writ_Bus(0x01,VH,display_transfer_mode);
	LCD_Writ_Bus(0x01,VL,display_transfer_mode);
}

void UTFT::LCD_Write_DATA(char VL)
{
	LCD_Writ_Bus(0x01,VL,display_transfer_mode);
}

void UTFT::LCD_Write_COM_DATA(char com1,int dat1)
{
     LCD_Write_COM(com1);
     LCD_Write_DATA(dat1>>8,dat1);
}

void UTFT::InitLCD(byte orientation)
{
	orient=orientation;

	sbi(P_RST, B_RST);
	delay(5); 
	cbi(P_RST, B_RST);
	delay(15);
	sbi(P_RST, B_RST);
	delay(15);

	cbi(P_CS, B_CS);

	switch(display_model)
	{
	case ILI9341:
		LCD_Write_COM(0xCB);  
		LCD_Write_DATA(0x39); 
		LCD_Write_DATA(0x2C); 
		LCD_Write_DATA(0x00); 
		LCD_Write_DATA(0x34); 
		LCD_Write_DATA(0x02); 

		LCD_Write_COM(0xCF);  
		LCD_Write_DATA(0x00); 
		LCD_Write_DATA(0XC1); 
		LCD_Write_DATA(0X30); 

		LCD_Write_COM(0xE8);  
		LCD_Write_DATA(0x85); 
		LCD_Write_DATA(0x00); 
		LCD_Write_DATA(0x78); 

		LCD_Write_COM(0xEA);  
		LCD_Write_DATA(0x00); 
		LCD_Write_DATA(0x00); 

		LCD_Write_COM(0xED);  
		LCD_Write_DATA(0x64); 
		LCD_Write_DATA(0x03); 
		LCD_Write_DATA(0X12); 
		LCD_Write_DATA(0X81); 

		LCD_Write_COM(0xF7);  
		LCD_Write_DATA(0x20); 

		LCD_Write_COM(0xC0);    //Power control 
		LCD_Write_DATA(0x23);   //VRH[5:0] 

		LCD_Write_COM(0xC1);    //Power control 
		LCD_Write_DATA(0x10);   //SAP[2:0];BT[3:0] 

		LCD_Write_COM(0xC5);    //VCM control 
		LCD_Write_DATA(0x3e); //?????
		LCD_Write_DATA(0x28); 

		LCD_Write_COM(0xC7);    //VCM control2 
		LCD_Write_DATA(0x86);  //--

		LCD_Write_COM(0x36);    // Memory Access Control 
		LCD_Write_DATA(0x48); //C8	   //48 68??//28 E8 ??

		LCD_Write_COM(0x3A);    
		LCD_Write_DATA(0x55); 

		LCD_Write_COM(0xB1);    
		LCD_Write_DATA(0x00);  
		LCD_Write_DATA(0x18); 

		LCD_Write_COM(0xB6);    // Display Function Control 
		LCD_Write_DATA(0x08); 
		LCD_Write_DATA(0x82);
		LCD_Write_DATA(0x27);  

		LCD_Write_COM(0xF2);    // 3Gamma Function Disable 
		LCD_Write_DATA(0x00); 

		LCD_Write_COM(0x26);    //Gamma curve selected 
		LCD_Write_DATA(0x01); 

		LCD_Write_COM(0xE0);    //Set Gamma 
		LCD_Write_DATA(0x0F); 
		LCD_Write_DATA(0x31); 
		LCD_Write_DATA(0x2B); 
		LCD_Write_DATA(0x0C); 
		LCD_Write_DATA(0x0E); 
		LCD_Write_DATA(0x08); 
		LCD_Write_DATA(0x4E); 
		LCD_Write_DATA(0xF1); 
		LCD_Write_DATA(0x37); 
		LCD_Write_DATA(0x07); 
		LCD_Write_DATA(0x10); 
		LCD_Write_DATA(0x03); 
		LCD_Write_DATA(0x0E); 
		LCD_Write_DATA(0x09); 
		LCD_Write_DATA(0x00); 

		LCD_Write_COM(0XE1);    //Set Gamma 
		LCD_Write_DATA(0x00); 
		LCD_Write_DATA(0x0E); 
		LCD_Write_DATA(0x14); 
		LCD_Write_DATA(0x03); 
		LCD_Write_DATA(0x11); 
		LCD_Write_DATA(0x07); 
		LCD_Write_DATA(0x31); 
		LCD_Write_DATA(0xC1); 
		LCD_Write_DATA(0x48); 
		LCD_Write_DATA(0x08); 
		LCD_Write_DATA(0x0F); 
		LCD_Write_DATA(0x0C); 
		LCD_Write_DATA(0x31); 
		LCD_Write_DATA(0x36); 
		LCD_Write_DATA(0x0F); 

		LCD_Write_COM(0x11);    //Exit Sleep 
		delay(120); 

		LCD_Write_COM(0x29);    //Display on 
		LCD_Write_COM(0x2c); 
		break;
	}
	sbi (P_CS, B_CS); 

	setColor(255, 255, 255);
	setBackColor(0, 0, 0);
	cfont.font=0;
}

void UTFT::setXY(word x1, word y1, word x2, word y2)
{
	if (orient==LANDSCAPE)
	{
		swap(word, x1, y1);
		swap(word, x2, y2)
		y1=disp_y_size-y1;
		y2=disp_y_size-y2;
		swap(word, y1, y2)
	}

	switch(display_model)
	{
		case ILI9341:
			LCD_Write_COM(0x2a); 
			LCD_Write_DATA(x1>>8);
			LCD_Write_DATA(x1);
			LCD_Write_DATA(x2>>8);
			LCD_Write_DATA(x2);
			LCD_Write_COM(0x2b); 
			LCD_Write_DATA(y1>>8);
			LCD_Write_DATA(y1);
			LCD_Write_DATA(y2>>8);
			LCD_Write_DATA(y2);
			LCD_Write_COM(0x2c);
		break;
	}
}

void UTFT::clrXY()
{
	if (orient==PORTRAIT)
		setXY(0,0,disp_x_size,disp_y_size);
	else
		setXY(0,0,disp_y_size,disp_x_size);
}

void UTFT::clrScr()
{
	long i;
	
	cbi(P_CS, B_CS);
	clrXY();
	if (display_transfer_mode!=1)
		sbi(P_RS, B_RS);
	for (i=0; i<((disp_x_size+1)*(disp_y_size+1)); i++)
	{
		if (display_transfer_mode!=1)
			LCD_Writ_Bus(0,0,display_transfer_mode);
		else
		{
			LCD_Writ_Bus(1,0,display_transfer_mode);
			LCD_Writ_Bus(1,0,display_transfer_mode);
		}
	}
	sbi(P_CS, B_CS);
}


void UTFT::fillScr(byte r, byte g, byte b)
{
	long i;
	char ch, cl;
	
	ch=((r&248)|g>>5);
	cl=((g&28)<<3|b>>3);

	cbi(P_CS, B_CS);
	clrXY();
	if (display_transfer_mode!=1)
		sbi(P_RS, B_RS);
	for (i=0; i<((disp_x_size+1)*(disp_y_size+1)); i++)
	{
		if (display_transfer_mode!=1)
			LCD_Writ_Bus(ch,cl,display_transfer_mode);
		else
		{
			LCD_Writ_Bus(1,ch,display_transfer_mode);
			LCD_Writ_Bus(1,cl,display_transfer_mode);
		}
	}
	sbi(P_CS, B_CS);
}

void UTFT::setColor(byte r, byte g, byte b)
{
	fcolorr=r;
	fcolorg=g;
	fcolorb=b;
}

void UTFT::setBackColor(byte r, byte g, byte b)
{
	bcolorr=r;
	bcolorg=g;
	bcolorb=b;
}

void UTFT::setPixel(byte r,byte g,byte b)
{
	LCD_Write_DATA(((r&248)|g>>5),((g&28)<<3|b>>3));	// rrrrrggggggbbbbb
}

void UTFT::drawPixel(int x, int y)
{
	cbi(P_CS, B_CS);
	setXY(x, y, x, y);
	setPixel(fcolorr, fcolorg, fcolorb);
	sbi(P_CS, B_CS);
	clrXY();
}

void UTFT::printChar(byte c, int x, int y)
{
	byte i,ch;
	int j;
	word temp; 

	cbi(P_CS, B_CS);
  
	if (orient==PORTRAIT)
	{
		setXY(x,y,x+cfont.x_size-1,y+cfont.y_size-1);
	  
		temp=((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;
		for( j=0; j < ((cfont.x_size/8)*cfont.y_size); j++)
		{
			ch=pgm_read_byte(&cfont.font[temp]);
			for( i=0; i<8; i++)
			{   
				if((ch&(1<<(7-i)))!=0)   
				{
					setPixel(fcolorr, fcolorg, fcolorb);
				} 
				else
				{
					setPixel(bcolorr, bcolorg, bcolorb);
				}   
			}
			temp++;
		}
	}
	else
	{
		temp=((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;

		for(j=0;j<((cfont.x_size/8)*cfont.y_size);j+=(cfont.x_size/8))
		{
			setXY(x,y+(j/(cfont.x_size/8)),x+cfont.x_size-1,y+(j/(cfont.x_size/8)));
			for (int zz=(cfont.x_size/8)-1; zz>=0; zz--)
			{
				ch=pgm_read_byte(&cfont.font[temp+zz]);
				for(i=0;i<8;i++)
				{   
					if((ch&(1<<i))!=0)   
					{
						setPixel(fcolorr, fcolorg, fcolorb);
					} 
					else
					{
						setPixel(bcolorr, bcolorg, bcolorb);
					}   
				}
			}
			temp+=(cfont.x_size/8);
		}
	}
	sbi(P_CS, B_CS);
	clrXY();
}

void UTFT::rotateChar(byte c, int x, int y, int pos, int deg)
{
	byte i,j,ch;
	word temp; 
	int newx,newy;
	double radian;
	radian=deg*0.0175;  

	cbi(P_CS, B_CS);

	temp=((c-cfont.offset)*((cfont.x_size/8)*cfont.y_size))+4;
	for(j=0;j<cfont.y_size;j++) 
	{
		for (int zz=0; zz<(cfont.x_size/8); zz++)
		{
			ch=pgm_read_byte(&cfont.font[temp+zz]); 
			for(i=0;i<8;i++)
			{   
				newx=x+(((i+(zz*8)+(pos*cfont.x_size))*cos(radian))-((j)*sin(radian)));
				newy=y+(((j)*cos(radian))+((i+(zz*8)+(pos*cfont.x_size))*sin(radian)));

				setXY(newx,newy,newx+1,newy+1);
				
				if((ch&(1<<(7-i)))!=0)   
				{
					setPixel(fcolorr, fcolorg, fcolorb);
				} 
				else  
				{
					setPixel(bcolorr, bcolorg, bcolorb);
				}   
			}
		}
		temp+=(cfont.x_size/8);
	}
	sbi(P_CS, B_CS);
	clrXY();
}

void UTFT::print(char *st, int x, int y, int deg)
{
	int stl, i;

	stl = strlen(st);

	if (orient==PORTRAIT)
	{
	if (x==RIGHT)
		x=(disp_x_size+1)-(stl*cfont.x_size);
	if (x==CENTER)
		x=((disp_x_size+1)-(stl*cfont.x_size))/2;
	}
	else
	{
	if (x==RIGHT)
		x=(disp_y_size+1)-(stl*cfont.x_size);
	if (x==CENTER)
		x=((disp_y_size+1)-(stl*cfont.x_size))/2;
	}

	for (i=0; i<stl; i++)
		if (deg==0)
			printChar(*st++, x + (i*(cfont.x_size)), y);
		else
			rotateChar(*st++, x, y, i, deg);
}

void UTFT::print(String st, int x, int y, int deg)
{
	char buf[st.length()+1];

	st.toCharArray(buf, st.length()+1);
	print(buf, x, y, deg);
}

/* 
void UTFT::printNumI(long num, int x, int y, int length, char filler)
{
	char buf[25];
	char st[27];
	boolean neg=false;
	int c=0, f=0;
  
	if (num==0)
	{
		if (length!=0)
		{
			for (c=0; c<(length-1); c++)
				st[c]=filler;
			st[c]=48;
			st[c+1]=0;
		}
		else
		{
			st[0]=48;
			st[1]=0;
		}
	}
	else
	{
		if (num<0)
		{
			neg=true;
			num=-num;
		}
	  
		while (num>0)
		{
			buf[c]=48+(num % 10);
			c++;
			num=(num-(num % 10))/10;
		}
		buf[c]=0;
	  
		if (neg)
		{
			st[0]=45;
		}
	  
		if (length>(c+neg))
		{
			for (int i=0; i<(length-c-neg); i++)
			{
				st[i+neg]=filler;
				f++;
			}
		}

		for (int i=0; i<c; i++)
		{
			st[i+neg+f]=buf[c-i-1];
		}
		st[c+neg+f]=0;

	}

	print(st,x,y);
}

void UTFT::printNumF(double num, byte dec, int x, int y, char divider, int length, char filler)
{
	char buf[25];
	char st[27];
	boolean neg=false;
	int c=0, f=0;
	int c2, mult;
	unsigned long inum;
  
	if (dec<1)
	dec=1;
	if (dec>5)
	dec=5;
	  
	if (num==0)
	{
		if (length!=0)
		{
			for (c=0; c<(length-2-dec); c++)
				st[c]=filler;
			st[c]=48;
			st[c+1]=divider;
			for (int i=0; i<dec; i++)
				st[c+2+i]=48;
			st[c+2+dec]=0;
		}
		else
		{
			st[0]=48;
			st[1]=divider;
			for (int i=0; i<dec; i++)
				st[2+i]=48;
			st[2+dec]=0;
		}
	}
	else
	{
		if (num<0)
		{
			neg=true;
			num=-num;
		}
	  
		mult = 1;
		for (int j = 0; j < dec; j++)
			mult = mult * 10;
		inum=long(num*mult + 0.5);
	  
		while (inum>0)
		{
			buf[c]=48+(inum % 10);
			c++;
			inum=(inum-(inum % 10))/10;
		}
		if ((num<1) and (num>0))
		{
			buf[c]=48;
			c++;
		}
		while (c<(dec+1))
		{
			buf[c]=48;
			c++;
		}
		buf[c]=0;
	  
		if (neg)
		{
			st[0]=45;
		}
	  
		if (length>(c+neg-1))
		{
			for (int i=0; i<(length-c-neg-1); i++)
			{
				st[i+neg]=filler;
				f++;
			}
		}

		c2=neg;
		for (int i=0; i<c; i++)
		{
			st[c2+f]=buf[c-i-1];
			c2++;
			if ((c-(c2-neg))==dec)
			{
				st[c2+f]=divider;
				c2++;
			}
		}
		st[c2+f]=0;
	}

	print(st,x,y);
}

*/

void UTFT::setFont(uint8_t* font)
{
	cfont.font=font;
	cfont.x_size=fontbyte(0);
	cfont.y_size=fontbyte(1);
	cfont.offset=fontbyte(2);
	cfont.numchars=fontbyte(3);
}

void UTFT::lcdOff()
{
	cbi(P_CS, B_CS);
	switch (display_model)
	{
	case PCF8833:
		LCD_Write_COM(0x28);
		break;
	}
	sbi(P_CS, B_CS);
}

void UTFT::lcdOn()
{
	cbi(P_CS, B_CS);
	switch (display_model)
	{
	case PCF8833:
		LCD_Write_COM(0x29);
		break;
	}
	sbi(P_CS, B_CS);
}

