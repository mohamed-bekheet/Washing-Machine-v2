/*
 * lcd20x4.h
 *
 * Created: 2/2/2021 11:26:51 AM
 *  Author: mohamed
 */ 


#ifndef _LCD20X4_H_
#define _LCD20X4_H_

//note : frequency of your cpu may affect


#include <avr/io.h>			/* Include AVR std. library file */
#include <util/delay.h>		/* Include Delay header file */
#include <stdlib.h>
#define LCD_Dir DDRD
#define LCD_Port PORTD

#define LCD_DIR_RS DDRB
#define LCD_PORT_RS PORTB

#define LCD_DIR_EN DDRC
#define LCD_PORT_EN PORTC
#define LCD_RS	PB3 //define MCU pin connected to LCD RS
#define LCD_EN	PC0	//define MCU pin connected to LCD E


// #define LCD_D0	0	//define MCU pin connected to LCD D0
// #define LCD_D1	1	//define MCU pin connected to LCD D1
// #define LCD_D2	2	//define MCU pin connected to LCD D1
// #define LCD_D3	3	//define MCU pin connected to LCD D2
#define LCD_D4	PD4	//define MCU pin connected to LCD D4
#define LCD_D5	PD5	//define MCU pin connected to LCD D5
#define LCD_D6	PD6	//define MCU pin connected to LCD D6
#define LCD_D7	PD7	//define MCU pin connected to LCD D7

#define LCD_LINE1 0x80
#define LCD_LINE2 0xC0
#define LCD_LINE3 0x94
#define LCD_LINE4 0xD4

void LCD_Command( unsigned char);
void LCD_Char( unsigned char );
void LCD_Init (void);
void LCD_String (char *);
void LCD_Number2_xy(uint8_t row,uint8_t pos,uint8_t num);
void LCD_Number3_xy (uint8_t X,uint8_t Y,uint8_t num);
void LCD_Number5_xy (uint8_t X,uint8_t Y,uint16_t num);
void LCD_String_xy (char , char , char *);
void LCD_Clear();



#endif /* LCD20X4_H_ */