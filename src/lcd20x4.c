
#include "lcd20x4.h"

void LCD_Command( unsigned char cmnd )
{
	//last 4 pins in port
	LCD_Port = (LCD_Port & 0x0F) | (cmnd & 0xF0); //sending upper nibble 
	LCD_PORT_RS &= ~ (1<<LCD_RS);		// RS=0, command reg. //
	LCD_PORT_EN |= (1<<LCD_EN);		    // Enable pulse //
	_delay_us(1);
	LCD_PORT_EN &= ~ (1<<LCD_EN);
	_delay_us(200);
	LCD_Port = (LCD_Port & 0x0F) | (cmnd << 4);  // sending lower nibble 
	LCD_PORT_EN |= (1<<LCD_EN);
	_delay_us(1);
	LCD_PORT_EN &= ~ (1<<LCD_EN);
	_delay_ms(2);
	/*
	if(cmnd& 0x80)  LCD_Port|=(1<<LCD_D7); else LCD_Port&= ~(1<<LCD_D7);
	if(cmnd & 0x40) LCD_Port|=(1<<LCD_D6); else LCD_Port&= ~(1<<LCD_D6);
	if(cmnd & 0x20) LCD_Port|=(1<<LCD_D5); else LCD_Port&= ~(1<<LCD_D5);
	if(cmnd & 0x10) LCD_Port|=(1<<LCD_D4); else LCD_Port&= ~(1<<LCD_D4);
	LCD_PORT_RS &=~(1<<LCD_RS);LCD_PORT_EN |=(1<<LCD_EN);
	_delay_ms(2);
	LCD_PORT_EN &=~(1<<LCD_EN);
	
	_delay_ms(2);
	
	if(cmnd & 0x08) LCD_Port|=(1<<LCD_D7); else LCD_Port&= ~(1<<LCD_D7);
	if(cmnd & 0x04) LCD_Port|=(1<<LCD_D6); else LCD_Port&= ~(1<<LCD_D6);
	if(cmnd & 0x02) LCD_Port|=(1<<LCD_D5); else LCD_Port&= ~(1<<LCD_D5);
	if(cmnd & 0x01) LCD_Port|=(1<<LCD_D4); else LCD_Port&= ~(1<<LCD_D4);
	LCD_PORT_RS &=~(1<<LCD_RS);
	LCD_PORT_EN |=(1<<LCD_EN);
	_delay_ms(2);
	LCD_PORT_EN &=~(1<<LCD_EN);
	_delay_ms(2);
	*/
}


void LCD_Char( unsigned char data )
{
	
	LCD_Port = (LCD_Port & 0x0F) | (data & 0xF0); // sending upper nibble //
	LCD_PORT_RS |= (1<<LCD_RS);		// RS=1, data reg. //
	LCD_PORT_EN|= (1<<LCD_EN);
	_delay_us(1);
	LCD_PORT_EN &= ~ (1<<LCD_EN);
	_delay_us(200);
	LCD_Port = (LCD_Port & 0x0F) | (data << 4); // sending lower nibble //
	LCD_PORT_EN |= (1<<LCD_EN);
	_delay_us(1);
	LCD_PORT_EN &= ~ (1<<LCD_EN);
	_delay_ms(2);
	/*
		if(data& 0x80)  LCD_Port|=(1<<LCD_D7); else LCD_Port&= ~(1<<LCD_D7);
		if(data & 0x40) LCD_Port|=(1<<LCD_D6); else LCD_Port&= ~(1<<LCD_D6);
		if(data & 0x20) LCD_Port|=(1<<LCD_D5); else LCD_Port&= ~(1<<LCD_D5);
		if(data & 0x10) LCD_Port|=(1<<LCD_D4); else LCD_Port&= ~(1<<LCD_D4);
		LCD_PORT_RS |=(1<<LCD_RS);LCD_PORT_EN |=(1<<LCD_EN);
		_delay_ms(2);
		LCD_PORT_EN &=~(1<<LCD_EN);
		
		_delay_ms(2);
		
		if(data & 0x08) LCD_Port|=(1<<LCD_D7); else LCD_Port&= ~(1<<LCD_D7);
		if(data & 0x04) LCD_Port|=(1<<LCD_D6); else LCD_Port&= ~(1<<LCD_D6);
		if(data & 0x02) LCD_Port|=(1<<LCD_D5); else LCD_Port&= ~(1<<LCD_D5);
		if(data & 0x01) LCD_Port|=(1<<LCD_D4); else LCD_Port&= ~(1<<LCD_D4);
		LCD_PORT_RS |=(1<<LCD_RS);
		LCD_PORT_EN |=(1<<LCD_EN);
		_delay_ms(2);
		LCD_PORT_EN &=~(1<<LCD_EN);
		_delay_ms(2);
		*/
}
void LCD_Init (void)			// LCD Initialize function //
{
	LCD_Dir  |= (0b11110000);			/* Make LCD port direction as o/p */
	LCD_DIR_EN |=(1<<LCD_EN);//OUTPUT
	LCD_DIR_RS |=(1<<LCD_RS);//OUTPUT
	_delay_ms(50);			/* LCD Power ON delay always >15ms */
	LCD_Command(0x01);
	LCD_Command(0x02);		        /* send for 4 bit initialization of LCD  */
	LCD_Command(0x28);              /* 4-bit mode */
	LCD_Command(0x0c);              /* Display on cursor off*/
	LCD_Command(0x06);              /* Increment cursor (shift cursor to right)*/
	LCD_Command(0x01);              /* Clear display screen*/;
	LCD_Command(0x80);             //set cursor first line
	_delay_ms(2);
}

void LCD_Number2_xy ( uint8_t row,uint8_t pos,uint8_t num)	//display 2 digits	/* Send string to LCD function */
{
	if (row == 0 && pos<20)
	LCD_Command((pos & 0xFF)+0x80);	/* Command of first row and required position<20 */
	else if (row == 1 && pos<20)
	LCD_Command((pos & 0xFF)+0xC0);	/* Command of first row and required position<20 */
	else if (row == 2 && pos<20)
	LCD_Command((pos & 0xFF)+(0x94));	/* Command of first row and required position<20 */
	else if (row == 3 && pos<20)
	LCD_Command((pos & 0xFF)+0xD4);	/* Command of first row and required position<20 */
	uint8_t snum [2] = {' '};	/* Command of first row and required position<16 */
	itoa(num,snum,10);
	for (uint8_t i = 0;i<2;i++)
	{
		if (snum[i]<'0'||snum[i]>'9')LCD_Char(' ');
		else LCD_Char(snum[i]);
	}
}
void LCD_Number3_xy ( uint8_t row,uint8_t pos,uint8_t num)	//display 3 digits	/* Send string to LCD function */
{
	if (row == 0 && pos<20)
	LCD_Command((pos & 0xFF)+0x80);	/* Command of first row and required position<20 */
	else if (row == 1 && pos<20)
	LCD_Command((pos & 0xFF)+0xC0);	/* Command of first row and required position<20 */
	else if (row == 2 && pos<20)
	LCD_Command((pos & 0xFF)+(0x94));	/* Command of first row and required position<20 */
	else if (row == 3 && pos<20)
	LCD_Command((pos & 0xFF)+0xD4);	/* Command of first row and required position<20 */
	uint8_t snum [3] = {' '};
	itoa(num,snum,10);
	for (uint8_t i = 0;i<3;i++)
	{
		if (snum[i]<'0'||snum[i]>'9')LCD_Char(' ');
		else LCD_Char(snum[i]);
		
	}
}
void LCD_Number5_xy ( uint8_t row,uint8_t pos,uint16_t num)	//display 5 digits	/* Send string to LCD function */
{
	if (row == 0 && pos<20)
	LCD_Command((pos & 0xFF)+0x80);	/* Command of first row and required position<20 */
	else if (row == 1 && pos<20)
	LCD_Command((pos & 0xFF)+0xC0);	/* Command of first row and required position<20 */
	else if (row == 2 && pos<20)
	LCD_Command((pos & 0xFF)+(0x94));	/* Command of first row and required position<20 */
	else if (row == 3 && pos<20)
	LCD_Command((pos & 0xFF)+0xD4);	/* Command of first row and required position<20 */
	uint8_t snum [5] = {' '};
	itoa(num,snum,10);
	for (uint8_t i = 0;i<5;i++)
	{
		if (snum[i]<'0'||snum[i]>'9')LCD_Char(' ');
		else LCD_Char(snum[i]);
		
	}
	

}
void LCD_String (char *str)		/* Send string to LCD function */
{
	int i;
	for(i=0;str[i]!=0;i++)		/* Send each char of string till the NULL */
	{
		LCD_Char (str[i]);
	}
}

void LCD_String_xy (char row, char pos, char *str)	/* Send string to LCD with xy position */
{
	if (row == 0 && pos<20)
	LCD_Command((pos & 0xFF)+0x80);	/* Command of first row and required position<20 */
	else if (row == 1 && pos<20)
	LCD_Command((pos & 0xFF)+0xC0);	/* Command of first row and required position<20 */
	else if (row == 2 && pos<20)
	LCD_Command((pos & 0xFF)+(0x94));	/* Command of first row and required position<20 */
	else if (row == 3 && pos<20)
	LCD_Command((pos & 0xFF)+0xD4);	/* Command of first row and required position<20 */
		LCD_String(str);		/* Call LCD string function */
}

void LCD_Clear()
{
	LCD_Command (0x01);		/* Clear display */
	_delay_ms(2);
	LCD_Command (0x80);		/* Cursor at home position */
}
/*
 * lcd20x4.c
 *
 * Created: 2/2/2021 11:27:09 AM
 *  Author: mohamed
 */ 
