/*
 *Washing_machine_AVR.c
 *Created: 1/6/2021 12:39:27 PM
 *Author : Mohamed Moustafa Aly
 *0 >> register &=~(1<<bitname)  ,  1 >> register|=(1<<bitname)|(1<<bitname)  , *toggle >>  register~=(1<<bitname)
 */
#define F_CPU 8000000
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <stdlib.h>
//#include "MY_LCD_1604.h"
#include "lcd20x4.h"
#include "mainAUX.h"
// urgent input
#define door_limit_sw PB0
#define overload PB1
#define vibration PB2
// water levels input
#define l3_full PB5
#define l2_mid PB6
#define l1_low PB7
// Buttons
#define start PB4
#define up PD0
#define down PD1
// interrupt pin
#define stop PD2		   // int0
#define pause PD3		   // int1
#define STOP_ADDR (0x002)  // int 0
#define PAUSE_ADDR (0x004) // int 1

// ADC
#define temp_pin PA7 // read out of op amp using ADC to get temprature

// output
#define k1 PA0 // RELAYS
#define k2 PA1
#define k3 PA2
#define k4 PA3
#define k5 PA4 // motor speed 5
#define k6 PA5 // Heater RELAY
#define buzzer PC1
#define door_lock PC2 // NOTE TIHS CHANGED INTO PC3
#define pump PC3	  // NOTE TIHS CHANGED INTO PC2
#define s1 PC4
#define s2 PC5
#define s3 PC6
#define s4 PC7

// functions declaration
void initialize_pins(void);
void tones(uint8_t tone);
void calculate_time(void);
void display_details(void);
void start_timer0(void);
void start_timer1(void);
void stop_timer0(void);
void stop_timer1(void);
void read_buttons(void);
// sub programs
void Prewash(void);
void Wash(void);
void Rinse(void);
void end(void);
void handle_errors(void);
int read_temperature(void);
void adjust_temprature(uint8_t temp_default, uint8_t temp_now); // get temprature needed and handle it with sensor and heater
void (*stop_isr)(void) = (void *)STOP_ADDR;						// call ISR for int1
void (*pause_isr)(void) = (void *)PAUSE_ADDR;					// call ISR for int1

static uint16_t heater_timer = 0;
volatile uint16_t counter_0 = 0;	   // max 65,536
volatile uint16_t counter_1 = 0;	   // max 65,536
volatile uint16_t counter_2 = 0;	   // max 65,536
volatile uint16_t counter_p = 0;	   // max 65,536 //used in pause ISR for debouncing
volatile uint16_t heating_counter = 0; // max 65,536 max delay needed is 20min>>60096
volatile uint16_t heating_timer = 0;   // max 65,536 max delay needed is 20min>>60096
volatile uint16_t water_counter = 0;   // used to check water level every specific time
volatile uint8_t stage = 1;
volatile uint8_t loopA_stage1 = 8; // for change motor direction
volatile uint8_t loopB_stage1 = 0; // for change motor direction
volatile bool stage_flag = 0;	   // will be set after every stage process
bool step_flag = 0;				   // used in prewash,wash,rinse
// use to call start timer one time not in every if loop check
uint8_t program = 0; // prewash by default
char program_name[] = " ";
bool start_button_pressed = 0;
bool program_selected_flag = 0; // will be set when press on start button
bool end_flag = 0;
bool change_step_flag = 0; // indicate that current step changed and must initialize new timer,//use to call start timer one time not in every if loop check
bool pause_flag = 0;
bool stop_flag = 0;
bool pump_error = 0;

static uint16_t t = 0;
volatile uint8_t needed_temp = 0;		   // temperature
volatile uint8_t current_temp = 0;		   // temperature
volatile bool heating_process = 0;		   // used to start heat adjust in program loop and set and cleared from timer0.
volatile bool heating_process_success = 0; // used to start heat adjust in program loop and set and cleared from timer0.
// volatile uint16_t heater_timer =0 ;//used to define when to raise heater error
volatile bool s2_s3_start_flag = 1;
uint8_t total_steps = 0;  // for main programs
uint8_t current_step = 1; // foe main programs
uint8_t sub_current_step = 0;
uint8_t sub_total_steps = 0;
bool sub_change_step_flag = 0;
bool l3_full_flag = 0;
bool l2_mid_flag = 0;
bool l1_low_flag = 0;

bool l3_full_start_check = 0; // to start timer check in timer2 for problems in water
bool l2_mid_start_check = 0;  // to check problem in water
bool l1_low_start_check = 0;  // problem with pump
bool details = 0;
uint32_t expected_time = 0; // store program time with 20ms and decrement from it in timer
long double time = 0;
uint8_t remaining_time_h = 0; // hours
uint8_t remaining_time_m = 0; // minutes
bool debuge_flag = 0;		  // used to display bugs and what i need while program is running
bool error_flag = 0;		  // default no error raised when error found
/////erorr flags
bool heating_sensor_error = 0; // indicate that there is no sensor
bool water_error = 0;
bool heat_error = 0;
bool clear_error = 0;
////fault flags
bool O_V_fault = 0;		  // overload;
bool D_limitsw_fault = 0; // door limit switch
bool Vib_fault = 0;		  // vibration
void initialize_pins(void)
{
	// INPUTS
	DDRB &= ~(1 << door_limit_sw); // do not pull up it
	//BUTTONS
	DDRB &= ~(1 << start);		   // start button
	PORTB |= (1 << start);		   // PULLUP
	DDRD &= ~(1 << up);			   // up button
	PORTD |= (1 << up);			   // PULLUP
	DDRD &= ~(1 << down);		   // down button
	PORTD |= (1 << down);		   // PULLUP
	DDRD &= ~(1 << stop);		   // stop button
	PORTD |= (1 << stop);		   // PULLUP
	DDRD &= ~(1 << pause);		   // pause button
	PORTD |= (1 << pause);		   // PULLUP
	
	DDRA &= ~(1 << temp_pin);	   // input ADC
	// outputs
	DDRA |= (1 << k1) | (1 << k2) | (1 << k3) | (1 << k4) | (1 << k5) | (1 << k6); // Contactors for motor speeds and heaters
	DDRC |= (1 << s1) | (1 << s2) | (1 << s3) | (1 << s4) | (1 << buzzer) | (1 << pump) | (1 << door_lock);
	// initialize ADC
	ADMUX |= (1 << ADLAR);								  // left adjust the result in register
	ADCSRA |= (1 << ADEN);								  // enable ADC
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // prescaler 128 to get max accurecy

	// Enable External interrupt
	// INT0,INT1
	// MCUCR |=(1<<ISC00)|(1<<ISC10);//any logical change for INT0 pin or INT1 pin
	// MCUCR |=(1<<ISC01)|(1<<ISC11);//falling edge
	MCUCR &= ~(1 << ISC00);
	MCUCR &= ~(1 << ISC01);
	MCUCR &= ~(1 << ISC10);
	MCUCR &= ~(1 << ISC11);			   // low level
	GICR |= (1 << INT0) | (1 << INT1); // INT0,INT1 ENABLE
	sei();							   // enable all interrupt request
}
void tones(uint8_t tone)
{
	if (tone == 0)
	{
		PORTC |= (1 << buzzer);
		_delay_ms(100);
		PORTC &= ~(1 << buzzer);
		_delay_ms(100);
		PORTC |= (1 << buzzer);
		_delay_ms(100);
		PORTC &= ~(1 << buzzer);
		_delay_ms(100);
		PORTC |= (1 << buzzer);
		_delay_ms(100);
		PORTC &= ~(1 << buzzer);
		_delay_ms(100);
		PORTC |= (1 << buzzer);
		_delay_ms(100);
		PORTC &= ~(1 << buzzer);
		_delay_ms(100);
		PORTC |= (1 << buzzer);
		_delay_ms(100);
		PORTC &= ~(1 << buzzer);
		_delay_ms(100);
		PORTC |= (1 << buzzer);
		_delay_ms(500);
		PORTC &= ~(1 << buzzer);
		_delay_ms(100);
	}
	else if (tone == 1) // error
	{
		PORTC |= (1 << buzzer);
		_delay_ms(300);
		PORTC &= ~(1 << buzzer);
		_delay_ms(90);
	}
}
void calculate_time(void)
{
	time = expected_time;
	time *= 0.019968;			  // convert to seconds
	time /= 60;					  // converted to minutes
	remaining_time_m = (int)time; // store minutes
	time /= 60;					  // convert to hours
	if (time < 1)
		time = 0;
	remaining_time_h = (int)time;
}
void display_details(void)
{
	
	// line0
	if (details)
	{
		LCD_String_xy(0, 0, "P");
		LCD_Number2_xy(0, 1, program);
		LCD_String_xy(0, 3, ":");
		if (program == 0)
		{
			LCD_String_xy(0, 4, "HOT    75*C"); // 11character
		}
		else if (program == 1)
		{
			LCD_String_xy(0, 4, "COLOUR 60*C");
		}
		else if (program == 2)
		{
			LCD_String_xy(0, 4, "WASH   40*C");
		}
		else if (program == 3)
		{
			LCD_String_xy(0, 4, "COLD    0*C");
		}
		else if (program > 2)
		{
			LCD_String_xy(0, 4, "...........");
		}

		LCD_String_xy(0, 15, "/");
		if (current_temp > 150)
			LCD_String_xy(0, 16, "**");
		else
			LCD_Number2_xy(0, 16, current_temp);
		LCD_String("*C");
		// LINE 1
		LCD_String_xy(1, 0, "S:");
		LCD_Number2_xy(1, 2, total_steps);
		LCD_String_xy(1, 4, "/");
		LCD_Number2_xy(1, 5, current_step);
		LCD_String_xy(1, 7, "             ");
		// line 2 state + errors
		if (pause_flag && !error_flag && !water_error && !heat_error)
		{
			LCD_String_xy(2, 0, "PAUSED  ||          "); // 10 character and 6 for errors sign
		}
		else if (end_flag && !error_flag && !water_error && !heat_error) // program finished
		{
			LCD_String_xy(2, 0, "FINISHED  :)        "); // 10 character and 6 for errors sign
		}
		else if (program_selected_flag && !error_flag && !water_error && !heat_error) // program started
		{
			LCD_String_xy(2, 0, "RUNNING  >>         "); // 10 character and 6 for errors sign
		}
		else if (!program_selected_flag && !end_flag && !error_flag) // program started
		{
			LCD_String_xy(2, 0, "CHOOSE   ^_^        "); // 10 character and 6 for errors sign
		}
		if (debuge_flag)
		{

			LCD_Number2_xy(3, 0, stage);
			LCD_Number2_xy(3, 2, sub_current_step);
			LCD_Number2_xy(3, 4, sub_change_step_flag);
			LCD_Number2_xy(3, 6, error_flag);
			LCD_Number2_xy(3, 8, heat_error);
			LCD_Number2_xy(3, 10, stage_flag);
			LCD_Number2_xy(3, 12, pause_flag);
			LCD_Number5_xy(3, 14, heating_counter);
		}
		else
		{ // timing system display remaining time
			LCD_String_xy(3, 0, "               ");
			LCD_Number2_xy(3, 15, remaining_time_h); // hours
			LCD_String_xy(3, 17, ":");
			LCD_Number2_xy(3, 18, remaining_time_m); // minutes
		}
	}
	else if ((error_flag || water_error || heat_error) && !details)
	{
		LCD_String_xy(0, 0, ".   !! ERROR !!    ."); // 20 CHAR
		if (heating_sensor_error)
		{
			LCD_String_xy(1, 0, "  NO HEAT SENSOR..  ");
		}
		else if (heat_error)
		{
			LCD_String_xy(1, 0, "    HEATER ERROR    ");
		}
		else
			LCD_String_xy(1, 0, "                    ");
		if (D_limitsw_fault)
		{
			LCD_String_xy(2, 0, "    CLOSE DOOR :)   ");
		}
		else if (O_V_fault)
		{
			LCD_String_xy(2, 0, "    OVER LOAD...   ");
		}
		else
			LCD_String_xy(2, 0, "                    ");
		if (water_error)
		{
			LCD_String_xy(3, 0, "    WATER ERROR     ");
		}
		else if (pump_error)
		{
			LCD_String_xy(3, 0, "     PUMP ERROR    ");
		}
		else if (Vib_fault)
		{
			LCD_String_xy(3, 0, "     VIBRATION     ");
		}
		else
			LCD_String_xy(3, 0, "                    ");
	}
}
int read_temperature(void)
{
	float t = 0;
	ADMUX = (7 << MUX0);   // ADC pin7 write pin number you want from 0 to 7
	ADCSRA |= (1 << ADSC); // start conversion
	while (!(ADCSRA & (1 << ADIF)))
		; // wait to get reading
	ADCSRA |= (1 << ADIF);
	t = ADC;
	t *= 5000;
	t /= 1023; // get output voltage of amplifier
	// gain of op amp 10
	// reference voltage of op amp is 5000/11
	t = (5000 / 11) + (t / 10);	   // vout = gain*(Vpt_100-vref)
	t = (1000 / ((5000 / t) - 1)); // actual value of sensor resistance
	t = (t - 100) / (0.385);
	// t+=0.5;//for debouncing
	return (int)t; // change global variable
}
void adjust_temprature(uint8_t temp_default, uint8_t temp_now)
{
	// pass needed_temperature variable which changed in timer2
	if (temp_now < temp_default)
	{
		PORTA |= (1 << k6);
	}
	else
	{
		PORTA &= ~(1 << k6); // normal case heater is turned off
	}
}
void handle_errors()
{										// do not put while loops here
	if ((~PINB) & (1 << door_limit_sw)) // read inputs if low it is error, must be high
	{									// if 0 take action
		error_flag = 1;
		D_limitsw_fault = 1;
		(*pause_isr)(); // clear error in pause
	}
	else
	{
		error_flag = 0;
		D_limitsw_fault = 0;
	}
	if ((~PINB) & (1 << overload)) // read inputs if low it is error, must be high
	{							   // if 0 take action
		error_flag = 1;
		O_V_fault = 1;
		(*pause_isr)(); // clear error in pause
	}
	else
	{
		error_flag = 0;
		O_V_fault = 0;
	}
	if ((~PINB) & (1 << vibration)) // read inputs if low it is error, must be high
	{								// if 0 take action
		error_flag = 1;
		Vib_fault = 1;
		(*pause_isr)(); // clear error in pause
	}
	else
	{
		error_flag = 0;
		Vib_fault = 0;
	}
	if (water_error)
	{
		water_error = 1;
		(*pause_isr)(); // clear error in pause
	}
	if (heat_error)
	{
		// error_flag = 1;//to break main while loop
		heat_error = 1;
		(*pause_isr)(); // clear error in pause
	}
	if (current_temp > 150 && needed_temp != 0)
	{
		error_flag = 1;
		heating_sensor_error = 1;
		(*pause_isr)();
	}
}
void end(void)
{
	end_flag = 1;
	// make sure that all outputs is off
	//......................
	// play buzzer using delay or not
	//	..............
	// dispaly message for finish program
	//............
	// open door lock
	//............
}
void stop_timer0(void)
{
	counter_0 = 0;
	TIMSK &= ~(1 << OCIE0); // stop timer0
}
void start_timer0(void) // 20ms
{
	TCCR0 |= (1 << CS02) | (1 << CS00) | (1 << WGM01); // CTC mode,Prescaler 1024
	TIMSK |= (1 << OCIE0);							   // enable CTC interrupt
	// max time can be got >>>T=((prescaler*(1+OCRn)/F_CPU) ,here max OCR0 is 255
	// Toverall = ((1024*(1+255)/8000000) = 32.768 mS
	// to get acurate calculations for 1sec,5sec,10sec  chose t=20ms
	// OCR0=((20ms*8000000)/1024)-1 = 155.25
	// OCR0 =155; //overflow every 0.019968sec>>19.968ms
	OCR0 = 250; // overflow every 0.032768
}
void start_timer1(void) // 50ms
{
	PORTA = 0x00; // turn off all pins used
	// turn off all output pins else door lock
	PORTC &= ~(1 << s1);
	PORTC &= ~(1 << s2);
	PORTC &= ~(1 << s3);
	PORTC &= ~(1 << s4);
	PORTC &= ~(1 << pump);
	// PORTC&=~(1<<door_lock);//in stop timer only
	loopA_stage1 = 1;
	counter_1 = 0;
	// TCCR1A no need to it now
	TCCR1B |= (1 << CS11) | (1 << CS10) | (1 << WGM12); // Prescaler 64 to avoid fractions,CTC mode4
	// TCCR1B |=(1<<CS12)|(1<<CS10)|(1<<WGM12);// prescaler 1024
	// there are two CTC channels timer in timer1
	// OCR1A,OCR1B
	TIMSK |= (1 << OCIE1A); // start timer ACTIVATE CHANNEL A
	// max time can be got >>>T=((prescaler*(1+OCRn)/F_CPU) ,here max OCR1A is 65535
	// Toverall = ((64*(1+65535)/8000000) = 1.045 sec
	// to get accurate calculations for 1sec,5sec,10sec  chose t=50ms
	// OCR1A=((50ms*8000000)/64)-1 = 6249
	OCR1A = 6249; // every overflow go to ISR timer1 get time overflow 0.050048sec>>>50.048msec
}
void stop_timer1(void)
{
	details = 1;
	PORTA = 0x00; // turn off all pins used
	PORTC &= ~(1 << s1);
	PORTC &= ~(1 << s2);
	PORTC &= ~(1 << s3);
	PORTC &= ~(1 << s4);
	PORTC &= ~(1 << pump);
	PORTC &= ~(1 << door_lock);
	// turn off all output pins
	counter_1 = 0;
	TIMSK &= ~(1 << OCIE1A); // stop_timer
}

void read_buttons(void) // timer2
{
	TIMSK &= ~(1 << OCIE2);							   // stop timer2
	TCCR2 |= (1 << CS02) | (1 << CS00) | (1 << WGM01); // CTC mode,Prescaler 1024
	TIMSK |= (1 << OCIE2);							   // enable CTC interrupt
	// max time can be got >>>T=((prescaler*(1+OCRn)/F_CPU) ,here max OCR0 is 255
	// Toverall = ((1024*(1+255)/8000000) = 32.768 mS
	OCR2 = 255;
}
uint8_t s = 0;
ISR(INT0_vect) // stop  //address to address is 0x002
{
	s++;
	if (s > 20) // for debouncing
	{
		s = 0;
		stop_flag = 1; // to finish any program
		program_selected_flag = 0;
		end_flag = 1;
		// clear all errors
		error_flag = 0; // break all error
		pause_flag = 0;
		water_error = 0;
		heat_error = 0;
		heating_process = 0;
		heating_process_success = 0;
		details = 1;
		current_step = 1;
		// stop all timers except timer 2
		stop_timer1();
	}
}

uint8_t p = 0;
ISR(INT1_vect) // pause //address to address is 0x004
{
	p++;
	if (p > 50) // for debouncing
	{
		debuge_flag = 1;
		if (program_selected_flag || error_flag) // mean that program is running or auto paused because of error
		{
			pause_flag = 1;
			// turn off all outputs unless door lock
			TIMSK &= ~(1 << OCIE1A); // stop_timer
			PORTA = 0x00;
			PORTC &= ~(1 << s1);
			PORTC &= ~(1 << s2);
			PORTC &= ~(1 << s3);
			PORTC &= ~(1 << s4);
			PORTC &= ~(1 << pump);
			while (pause_flag)
			{
				counter_p++;
				// LCD details for errors and states
				display_details();
				if (counter_p > 5)
				{
					counter_p = 0;
					if ((~PIND) & (1 << up)) // when button pressed
					{
						// change steps
						current_step++;
						change_step_flag = 1;
						if (current_step == 0 || current_step > total_steps)
							current_step = 1;
					}
					if ((~PIND) & (1 << down)) // when button pressed
					{
						// change steps
						current_step--;
						change_step_flag = 1;
						if (current_step == 0)
							current_step = 1;
					}
				}
				if ((~PIND) & (1 << stop))
				{
					details = 1;
					stop_timer1();
					(*stop_isr)();
				}
				if ((~PINB) & (1 << door_limit_sw)) // read inputs if low it is error, must be high
				{
					// if 0 take action
					error_flag = 1;
					D_limitsw_fault = 1;
					details = 0;
				}
				else
				{
					D_limitsw_fault = 0;
				}
				if ((~PINB) & (1 << overload)) // read inputs if low it is error, must be high
				{							   // if 0 take action
					error_flag = 1;
					O_V_fault = 1;
					details = 0;
				}
				else
				{
					O_V_fault = 0;
				}
				if ((~PINB) & (1 << vibration)) // read inputs if low it is error, must be high
				{								// if 0 take action
					error_flag = 1;
					Vib_fault = 1;
					details = 0;
				}
				else
				{
					Vib_fault = 0;
				}
				if (current_temp > 150 && needed_temp != 0)
				{
					error_flag = 1;
					heating_sensor_error = 1;
					details = 0;
				}

				// water_flag raised
				if (water_error)
				{
					details = 0;
					if ((~PINB) & (1 << start)) // if pressed
					{
						details = 1;
						pause_flag = 0;
						// this error happens only in stage 1 in timer 0 thus we do not need to define stage variable again
						water_error = 0; // clear water error flag
						sub_change_step_flag = 1;
					}
					// tones(1);
				}
				if (!D_limitsw_fault && !O_V_fault && !Vib_fault && !water_error && !heat_error && !heating_sensor_error) // check for all errors to gether to clear global error
				{																										  // if no error clear flag
					error_flag = 0;
					details = 1;
				}
				if (heat_error)
				{
					// heat_error = 0;
					heating_process = 0;
					heating_process_success = 0;
					details = 0;
					// tones(1);//error sound
				}
				if (error_flag)
				{

					if (needed_temp != 0)
						current_temp = read_temperature();
					if (current_temp < 150)
						heating_sensor_error = 0;
					heat_error = 0;
					// tones(1);
				}
				else if ((((~PINB) & (1 << start)) && (!error_flag))) // break pausing
				{
					details = 1;
					// continue program do not save latest condition must start again from beginning of the stage
					// if(counter_0 > 2)counter_0 -=2 ;//if to avoid negative or overflowed numbers and to restore latest state
					// if(expected_time>2)expected_time-=2;
					counter_0 = 0;
					if (heating_counter > 2)
						heating_counter -= 2;
					start_timer1();
					// TIMSK|=(1<<OCIE0);//suppose water error happened but if not next line will handle it
					change_step_flag = 1; // to start stage from beginning again
					pause_flag = 0;
					break;
				}
			}
		}
	}
}
ISR(TIMER0_COMP_vect) // used for heating and water process
{
	// volatile uint16_t c = heater_timer ;
	if (program_selected_flag)
	{
		if (l3_full_start_check || l2_mid_start_check)
			water_counter++;
		if (s2_s3_start_flag)
		{
			counter_0++;
			PORTC |= (1 << s2);
			PORTC |= (1 << s3);
		}

		if ((l3_full_flag && l3_full_start_check) || (l2_mid_flag && l2_mid_start_check))
		{ // then no error

			water_counter = 0;
			water_error = 0;

			// start new 6min:40sec*/
			sub_change_step_flag = 1; // used for prewash
			l3_full_start_check = 0;
			l2_mid_start_check = 0;
		}
		// 9155 >> 5 min
		// 1831 >> 1 min
		// 915 >> 30 sec
		// 305 >> 10sec
		else if (water_counter > 305 && (!l2_mid_flag || !l3_full_flag)) // 5min and no water then error
		{
			water_counter = 0;
			water_error = 1;
		}
		if (counter_0 > 305) // 10sec
		{
			counter_0 = 0;
			s2_s3_start_flag = 0;
			PORTC &= ~(1 << s2);
			PORTC &= ~(1 << s3);
		}

		if (heating_process)
		{
			heating_counter++;
			if (heating_counter > heater_timer) // 25min
			{
				heating_counter = 0;
				if ((current_temp >= (needed_temp - 7)) && (current_temp <= (needed_temp + 7)))
				{ // tare is no error
					heating_process = 0;
					heating_process_success = 1;
					heat_error = 0;
					needed_temp = 0; // to stop heater in the adjust_heat function
				}
				else
				{ // thus there is error with heater
					heating_process_success = 0;
					heat_error = 1; // handle error function see this flag
				}
			}
		}
	}
}
ISR(TIMER1_COMPA_vect)
{
	counter_1++;	// counter step every 20 count take 1 sec
	if (stage == 1) // Distribution >> k1,k2,s1 Change Motor Direction >>right and left
	{
		// 50 sec * 4 >> 200 sec total time
		if (counter_1 < 99)
		{
			// PORTA^=(1<<k1)|(1<<k2);
			PORTA &= ~(1 << k1);//off
			PORTA &= ~(1 << k2);//off
		}
		// 5 sec
		else if ((counter_1 >= 100) && (counter_1 < 105)) // 5 sec
		{
			//20
			PORTA &= ~(1 << k2);//off
			PORTA |= (1 << k1);//on
			if (!l3_full_flag)
				PORTC |= (1 << s1); // solenoid for water
		}
		// 5sec
		else if ((counter_1 >= 500) && (counter_1 < 505)) // 25 sec
		{
			//5sec
			// PORTA^=(1<<k1)|(1<<k2);
			PORTA &= ~(1 << k1);//off
			PORTA &= ~(1 << k2);//on
		}
		// 20 sec
		else if ((counter_1 >= 600) && (counter_1 < 605)) // 30 sec
		{
			//20
			PORTA &= ~(1 << k1);
			PORTA |= (1 << k2);
		}
		// 5sec
		else if (counter_1 >= 1000) // 50 sec
		{
			loopA_stage1--;
			counter_1 = 0; // new loop
		}
		if (loopA_stage1 == 0) // when loopA_stage1 reach 1 will finish 4 rounds >>>3min+20sec total 200 sec
		{
			// finish it
			counter_1 = 0;
			PORTA &= ~(1 << k1);
			PORTA &= ~(1 << k2);
			PORTC &= ~(1 << s1);
			stage_flag = 1;			 // successful stage
			TIMSK &= ~(1 << OCIE1A); // stop timer0
		}
		if ((l3_full_flag) || (l2_mid_flag && l2_mid_start_check))
		{
			PORTC &= ~(1 << s1);//off
		}
	}
	else if (stage == 2) // Drain >> k3,k4,pump
	{					 // sec >> 3min
		if (counter_1 < 600)
		{						 // will play for 120 sec
			PORTA &= ~(1 << k4); // Contactor 4 off
			PORTA |= (1 << k3);	 // Contactor 3 on
			PORTC &= ~(1 << pump);
		}
		// 10sec
		else if ((counter_1 >= 600) && (counter_1 < 605)) // pass 10sec
		{												  // open after K3 open with 30 sec
			PORTC |= (1 << pump);						  // open pump to release water
		}
		else if ((counter_1 >= 2400) && (counter_1 < 2405)) // pass 120sec
		{													// playing last for 60 sec
			if (l1_low_flag)
			{ // must be 0 to indicate there is no water
				pump_error = 1;
			}
			else
			{
				PORTA &= ~(1 << k3); // Contactor 3 off
				PORTA |= (1 << k4);	 // Contactor 4 on
			}
		}
		else if (counter_1 >= 3600) // pass 120+60>>180 sec
		{
			PORTA &= ~(1 << k4);
			PORTC &= ~(1 << pump);
			counter_1 = 0;
			stage_flag = 1;			 // RAISE FLAG AFTER FINISHING ROUND
			TIMSK &= ~(1 << OCIE1A); // stop timer0
		}
	}
	else if (stage == 3) // spin
	{					 // 350 sec >> 5min:50sec
		// do not start spin directly
		if (counter_1 < 200)
		{
			PORTA |= (1 << k5);
			PORTC |= (1 << pump);
			PORTC |= (1 << s4); // perfuming
		}
		else if ((counter_1 >= 200) && (counter_1 < 205)) // 10 sec
		{
			PORTC &= ~(1 << s4);
		}
		else if ((counter_1 >= 6000) && (counter_1 < 6005)) // 5min 300sec~320
		{
			PORTA &= ~(1 << k5);
			PORTC &= ~(1 << pump);
			PORTA &= ~(1 << k2);
			PORTA |= (1 << k1);
		}
		else if ((counter_1 >= 6400) && (counter_1 < 6405)) // 320sec~325
		{
			// PORTA^=(1<<k1)|(1<<k2);
			PORTA &= ~(1 << k1);
			PORTA &= ~(1 << k2);
		}
		else if ((counter_1 >= 6500) && (counter_1 < 6505)) // 325 sec~345
		{
			PORTA &= ~(1 << k1);
			PORTA |= (1 << k2);
		}
		// 5sec
		else if ((counter_1 >= 6900) && (counter_1 < 6905)) // 345 sec~350
		{
			PORTA &= ~(1 << k1);
			PORTA &= ~(1 << k2);
		}
		// 20 sec
		else if ((counter_1 >= 7000)) // 350 sec~.....
		{
			counter_1 = 0;
			stage_flag = 1;
			TIMSK &= ~(1 << OCIE1A); // stop timer
		}
	}
	else if (stage == 4) // slowdown and end
	{					 // wait 30 sec

		if (counter_1 > 400) // 20sec
		{
			counter_1 = 0;
			stage_flag = 1;
			TIMSK &= ~(1 << OCIE1A); // stop timer
		}
	}
	else if (stage == 5) // con 5 for 60 sec then water in for 60 sec
	{
		PORTA |= (1 << k5);
		if (counter_1 >= 1200)
		{
			PORTA &= ~(1 << k5);
			PORTC |= (1 << s1);
		} // 60 sec
		else if (counter_1 >= 2400)
		{
			PORTC &= ~(1 << s1);
			counter_1 = 0;
			stage_flag = 1;
			TIMSK &= ~(1 << OCIE1A);
		}
	}
}


ISR(TIMER2_COMP_vect) // reading buttons without delay and debouncing circuit
{
	counter_2++;				// used to make delay to overcome bouncing of switches
	if ((~PINB) & (1 << start)) // when button pressed
	{
		if ((~PINB) & (1 << door_limit_sw)) // read inputs if low it is error, must be high
		{									// if 0 take action
			error_flag = 1;
			D_limitsw_fault = 1;
			details = 0;
		}
		else if ((~PINB) & (1 << overload)) // read inputs if low it is error, must be high
		{									// if 0 take action
			error_flag = 1;
			O_V_fault = 1;
			details = 0;
		}
		else if ((~PINB) & (1 << vibration)) // read inputs if low it is error, must be high
		{									 // if 0 take action
			error_flag = 1;
			Vib_fault = 1;
			details = 0;
		}
		else if (!error_flag)
		{ // no problem
			program_selected_flag = 1;
			end_flag = 0;
			details = 1;
		}
		else
		{ // problem
			details = 0;
			error_flag = 1;
			program_selected_flag = 0; // do not start program
			(*pause_isr)();			   // pause
		}
		if ((PINB) & (1 << door_limit_sw))
		{
			D_limitsw_fault = 0;
		}
		if ((PINB) & (1 << overload))
		{
			O_V_fault = 0;
		}
		if ((PINB) & (1 << vibration))
		{
			Vib_fault = 0;
		}
	}
	if (counter_2 > 25)
	{
		counter_2 = 0;
		if (!program_selected_flag && !pause_flag && !error_flag)
		{
			if ((~PIND) & (1 << up)) // when button pressed
			{
				end_flag = 0;
				program++;
				if (program < 0 || program > 15)
					program = 0; // avoid negative numbers
			}
			if ((~PIND) & (1 << down)) // when button pressed
			{
				program--;
				end_flag = 0;
				if (program < 0 || program > 15)
					program = 0; // avoid negative numbers
			}
		}
	}

	if (PINB & (1 << l3_full))
		l3_full_flag = 1; // high
	else
		l3_full_flag = 0;
	if (PINB & (1 << l2_mid))
		l2_mid_flag = 1; // high
	else
		l2_mid_flag = 0;
	if (PINB & (1 << l1_low))
		l1_low_flag = 1; // high
	else
		l1_low_flag = 0;
	if (needed_temp != 0)
		current_temp = read_temperature();
}

// THOSE WILL BE REPLACED AFTER FINISHING ALL PROGRAMS
void Prewash(void)
{
	// 1140 sec >> 19 min
	sub_total_steps = 6;
	// start program sequence
	if (stage_flag && sub_current_step < sub_total_steps) // increment step after step finished and raise its flag(stage flag)
	{													  // auto increment step
		sub_current_step++;
		stage_flag = 0;
		// it is used to call start_timer0 one time
		// start button set it
		sub_change_step_flag = 1;
	}
	else if (sub_current_step == 1 && sub_change_step_flag) // 400
	{														// throw in only one time if sub_change_step_flag raised
		sub_change_step_flag = 0;
		stage = 1;		// 200sec
		start_timer1(); // must called only one time
		if (l3_full_flag && !water_error)
			l3_full_start_check = 0;
		else
			l3_full_start_check = 1;
	}
	else if (sub_current_step == 2 && sub_change_step_flag) // 180
	{
		sub_change_step_flag = 0;
		stage = 2; // 180 sec
		start_timer1();
		l3_full_start_check = 0;
	}
	// 6min:20sec
	else if (sub_current_step == 3 && sub_change_step_flag)
	{
		sub_change_step_flag = false;
		stage = 1;
		start_timer1();
		if (l3_full_flag && !water_error)
			l3_full_start_check = 0;
		else
			l3_full_start_check = 1;
	}
	else if (sub_current_step == 4 && sub_change_step_flag)
	{
		sub_change_step_flag = false;
		stage = 2;
		start_timer1();
		l3_full_start_check = 0;
	}
	else if (current_step == 5 && sub_change_step_flag)
	{
		sub_change_step_flag = 0;
		stage = 1;
		start_timer1();
		if (l3_full_flag && !water_error)
			l3_full_start_check = 0;
		else
			l3_full_start_check = 1;
	}
	else if (sub_current_step == 6 && sub_change_step_flag)
	{
		sub_change_step_flag = 0;
		stage = 2;
		start_timer1();
		l3_full_start_check = 0;
	}
	else if (stage_flag && sub_current_step == sub_total_steps)
	{ // raise flag to change main program step
		step_flag = 1;
	}
	if (l3_full_flag && !water_error && !l3_full_start_check)
		PORTC &= ~(1 << s1); // close solenoid
}

void Wash(void)
{

	sub_total_steps = 3;
	// start program sequence
	if (heating_process)
	{
		// pass needed_temperature variable which changed in timer2
		if (current_temp < needed_temp)
		{
			PORTA |= (1 << k6);
		}
		else
		{
			PORTA &= ~(1 << k6); // normal case heater is turned off
		}
	}
	if (sub_current_step == 1)
	{
		stage = 1;				// 200sec
		start_timer1();			// distribution //must called only one time
		l2_mid_start_check = 1; // to start timer for water
		sub_current_step++;
	}
	else if (sub_current_step == 2)
	{
		// wait here running distribution until level 2 flag raised
		sub_change_step_flag = 0;
		loopA_stage1 = 4; // still reset loop of distribution to work along heating process

		if (l2_mid_flag)
		{						  // initialize timer0 for heating
			PORTC &= ~(1 << s1);  // stop entering water
			heating_process = 1;  // to start calculate heat timers in timer0 heating_counter
			s2_s3_start_flag = 1; // play in timer0 for 10 sec
			sub_current_step++;	  // go to sub_current_step 3
		}
	}
	else if (sub_current_step == 3)
	{
		if (heating_process_success)
		{
			heating_process_success = 0;
			stage = 1;
			start_timer1();
			sub_current_step++;
		}
	}
	else if (sub_current_step == 4 && stage_flag)
	{
		stage = 1;
		start_timer1();
		sub_current_step++;
	}
	else if (sub_current_step == 5 && stage_flag)
	{
		stage = 2;
		start_timer1();
		sub_current_step++;
	}
	else if (current_step == 6 && step_flag)
	{
		step_flag = 1;
	}
}


int main(void)
{
	initialize_pins();
	_delay_ms(500); // wait to get successful initialization
	LCD_Init();
	read_buttons(); // timer2
	start_timer0(); // start timer 0
	end_flag = 0;
	sub_current_step = 1;
	sub_change_step_flag = 1;
	details = 1;
	PORTA |= (1 << k6);
	while (1)
	{
		debuge_flag = 0; // USED TO DISPALY FLAGS ON LCD
		display_details();
		if (program == 0) // wash 70 *C
		{
			// program details
			// 6 steps>>>stage1,2 *3times end process when all steps finished
			needed_temp = 75; // change for different program
			heater_timer = 100;
			total_steps = 2;	 // change for different program
			sub_total_steps = 7; // change for different program
			current_step = 1;
			step_flag = 0;
			change_step_flag = 1;
			sub_current_step = 1;
			sub_change_step_flag = 1;
			current_temp = 0;
			s2_s3_start_flag = 0;
			l2_mid_start_check = 0;
			l3_full_start_check = 0;
			expected_time = 0;
			end_flag = 0;
			heating_process = 0;
			while (!end_flag && program_selected_flag && !error_flag) // end its self or stop button raise  this flag&//start button pressed
			{
				details = 1;
				// start any program with thin lines
				////////////////////////////any_program////////////////////////////
				// if(!program_selected_flag||error_flag||end_flag)break;
				debuge_flag = 1;
				PORTC |= (1 << door_lock); // close door
				display_details();
				handle_errors();
				// calculate time
				long double time = expected_time;
				time *= 0.05;				  // convert to seconds
				time /= 60;					  // converted to minutes
				remaining_time_m = (int)time; // store minutes
				time /= 60;					  // convert to hours
				if (time < 1)
					time = 0;
				remaining_time_h = (int)time;
				if (step_flag && current_step < total_steps) // increment step after step finished and raise its flag(stage flag)
				{											 // auto increment step
					current_step++;
					step_flag = 0;
					// it is used to call start_timer0 one time
					// start button set it
					change_step_flag = 1;
				}
				////////////////////////////any_program////////////////////////////

				else if (current_step == 1) // 400
				{
					change_step_flag = 0;
					// start program sequence
					if (stage_flag && sub_current_step < sub_total_steps) // increment step after step finished and raise its flag(stage flag)
					{													  // auto increment step
						sub_current_step++;
						stage_flag = 0;
						// it is used to call start_timer0 one time
						// start button set it
						sub_change_step_flag = 1;
					}
					else if (sub_current_step == 1 && sub_change_step_flag) // 200
					{														// throw in only one time if sub_change_step_flag raised
						sub_change_step_flag = 0;
						stage = 1;		// 200sec
						start_timer1(); // must called only one time
						if (l3_full_flag && !water_error)
							l3_full_start_check = 0;
						else
							l3_full_start_check = 1;
					}
					else if (sub_current_step == 2 && sub_change_step_flag) // 180
					{
						sub_change_step_flag = 0;
						stage = 2; // 180 sec
						start_timer1();
						l3_full_start_check = 0;
					}
					// 6min:20sec
					else if (sub_current_step == 3 && sub_change_step_flag) // 200 sec
					{
						sub_change_step_flag = false;
						stage = 1;
						start_timer1();
						if (l3_full_flag && !water_error)
							l3_full_start_check = 0;
						else
							l3_full_start_check = 1;
					}
					else if (sub_current_step == 4 && sub_change_step_flag)
					{
						sub_change_step_flag = false;
						stage = 2;
						start_timer1();
						l3_full_start_check = 0;
					}
					if (current_step == 5 && sub_change_step_flag)
					{
						sub_change_step_flag = 0;
						stage = 1;
						start_timer1();
						if (l3_full_flag && !water_error)
							l3_full_start_check = 0;
						else
							l3_full_start_check = 1;
					}
					else if (sub_current_step == 6 && sub_change_step_flag)
					{
						sub_change_step_flag = 0;
						stage = 2;
						start_timer1();
						l3_full_start_check = 0;
					}
					else if (sub_current_step == 7 && sub_change_step_flag)
					{
						sub_change_step_flag = 0;
						stage = 5; // con5 60 sec , s1 60sec >>
						start_timer1();
						if (l2_mid_flag && !water_error)
							l2_mid_start_check = 0;
						else
							l2_mid_start_check = 1;
					}
					else if (stage_flag && sub_current_step == sub_total_steps)
					{ // raise flag to change main program step
						step_flag = 1;
						sub_current_step = 1; // reinitialize for next step
					}
					if (l3_full_flag && !water_error && !l3_full_start_check)
						PORTC &= ~(1 << s1); // close solenoid
				}
				else if (current_step == 2)
				{
					change_step_flag = 0;

					// start program sequence
					if (heating_process)
					{
						// pass needed_temperature variable which changed in timer2
						if ((current_temp <= (needed_temp - 3)))
						{
							PORTA |= (1 << k6);
						}
						else
						{
							PORTA &= ~(1 << k6); // normal case heater is turned off
						}
					}
					if (sub_current_step == 1)
					{
						stage = 1;				// 200sec
						start_timer1();			// distribution //must called only one time
						l2_mid_start_check = 1; // to start timer for water
						sub_current_step++;
					}
					else if (sub_current_step == 2)
					{
						// wait here running distribution until level 2 flag raised
						sub_change_step_flag = 0;
						loopA_stage1 = 4; // still reset loop of distribution to work along heating process

						if (l2_mid_flag)
						{						  // initialize timer0 for heating
							PORTC &= ~(1 << s1);  // stop entering water
							heating_process = 1;  // to start calculate heat timers in timer0 heating_counter
							s2_s3_start_flag = 1; // play in timer0 for 10 sec
							sub_current_step++;	  // go to sub_current_step 3
						}
					}
					else if (sub_current_step == 3)
					{
						loopA_stage1 = 4; // still reset loop of distribution to work along heating process
						if (heating_process_success)
						{
							heating_process_success = 0;
							stage = 1;
							start_timer1();
							sub_current_step++;
						}
					}
					else if (sub_current_step == 4 && stage_flag)
					{
						stage = 1;
						start_timer1();
						sub_current_step++;
					}
					else if (sub_current_step == 5 && stage_flag)
					{
						stage = 2;
						start_timer1();
						sub_current_step++;
					}
					else if (sub_current_step == 6 && stage_flag)
					{
						stage = 4;
						start_timer1();
						sub_current_step++;
					}
					else if (current_step == 7 && stage_flag)
					{
						change_step_flag = 0;
						step_flag = 1;
					}
				}
				else if (current_step == 3) // rinse
				{
				}

				////////////////////////finish_program_sequence//////////////////////
				/// these lines used in any program to finish it
				if (step_flag && !change_step_flag && current_step == total_steps)
				{ // finish program
					step_flag = 0;
					end_flag = 1;				// cleared in start button
					program_selected_flag = 0;	// cleared with start button for waiting start button press again to start program
					PORTC &= ~(1 << door_lock); // open_door
					stop_timer1();
					tones(0); // make sound after finishing
					// finish program
					break;
				}
			}
		}

		else if (program == 1) // wash with heat 40*C
		{
			// program details
			needed_temp = 40;
			total_steps = 2; // heating and drain
			current_step = 1;
			stage_flag = 0; // raised after stage finished its procedure
			change_step_flag = 1;
			while (!end_flag && program_selected_flag) // end its self or stop button raise  this flag&//start button pressed
			{
				PORTC |= (1 << door_lock); // close door
				display_details();
				handle_errors();
				// if(needed_temp!=0) read_temperature();
				// if(heating_process) adjust_temprature(needed_temp);
				// calculate time
				/*
				long double time = expected_time ;
				time*=0.019968;//convert to seconds
				time/=60;//converted to minutes
				remaining_time_m = (int)time;//store minutes
				time/=60;//convert to hours
				if(time < 1)time =0;
				remaining_time_h =(int)time;
				*/
				if (stage_flag && current_step < total_steps) // increment step after step finished and raise its flag(stage flag)
				{
					current_step++;
					stage_flag = 0;
					// change_step_flag is used to call start_timer0 one time
					change_step_flag = 1; // start button set it
				}
				else if (current_step == 1 && change_step_flag) // 400
				{
					// expected_time =87140;//29min > 1740 sec
					change_step_flag = 0;
					stage = 1;
					needed_temp = 40;
					start_timer1(); // must called only one time
				}
				else if (current_step == 2 && change_step_flag) // 180
				{
					// expected_time =67107 ;//1340 sec
					change_step_flag = 0;
					stage = 2;
					start_timer1();
				}
				else if (stage_flag && !change_step_flag && current_step == total_steps)
				{ // stop program
					stage_flag = 0;
					end_flag = 1;
					program_selected_flag = 0;	// for waiting start button press again to start program
					PORTC &= ~(1 << door_lock); // open_door
					tones(0);					// make sound after finishing
					// finish program
					break;
				}
			}
		}

		else if (program == 2) // rinse
		{
			// program details
			needed_temp = 0;
			total_steps = 7;
			current_step = 1;
			stage_flag = 0;
			change_step_flag = 1;
			current_temp = 0;
			while (!end_flag && program_selected_flag) // end its self or stop button raise  this flag&//start button pressed
			{
				PORTC |= (1 << door_lock); // close door
				display_details();
				handle_errors();

				// calculate time
				/*
				long double time = expected_time ;
				time*=0.019968;//convert to seconds
				time/=60;//converted to minutes
				remaining_time_m = (int)time;//store minutes
				time/=60;//convert to hours
				if(time < 1)time =0;
				remaining_time_h =(int)time;
				*/
				if (stage_flag && current_step < total_steps) // increment step after step finished and raise its flag(stage flag)
				{
					current_step++;
					stage_flag = 0;
					// it is used to call start_timer0 one time
					// start button set it
					change_step_flag = 1;
				}
				else if (current_step == 1 && change_step_flag) // 400
				{
					// expected_time =87140;//29min > 1740 sec
					change_step_flag = 0;
					stage = 1;
					start_timer1(); // must called only one time
				}
				else if (current_step == 2 && change_step_flag) // 180
				{
					// expected_time =67107 ;//1340 sec
					change_step_flag = 0;
					stage = 2;
					start_timer1();
				}
				else if (current_step == 3 && change_step_flag)
				{
					// expected_time =58092 ;//1160 sec
					change_step_flag = false;
					stage = 1;
					start_timer1();
				}
				else if (current_step == 4 && change_step_flag)
				{
					// expected_time =38061 ;//760 sec
					change_step_flag = false;
					stage = 2;
					start_timer1();
				}
				else if (current_step == 5 && change_step_flag)
				{
					expected_time = 29046; // 580
					change_step_flag = 0;
					stage = 1;
					start_timer1();
				}
				else if (current_step == 6 && change_step_flag)
				{
					// expected_time =9014;//180 sec
					// if(expected_time <1)expected_time=0;
					change_step_flag = 0;
					stage = 2;
					start_timer1();
				}
				else if (current_step == 7 && change_step_flag)
				{
					// expected_time =9014;//180 sec
					// if(expected_time <1)expected_time=0;
					change_step_flag = 0;
					stage = 3; // spin
					start_timer1();
				}
				else if (stage_flag && !change_step_flag && current_step == total_steps)
				{ // stop program
					stage_flag = 0;
					end_flag = 1;
					program_selected_flag = 0;	// for waiting start button press again to start program
					PORTC &= ~(1 << door_lock); // open_door
					tones(0);					// make sound after finishing
					// finish program
					break;
				}
			}
		}
		if (program == 3) // prewash
		{
			// program details
			// display_details();
			// 6 steps>>>stage1,2 *3times end process when all steps finished
			needed_temp = 0;
			total_steps = 6;
			current_step = 1;
			step_flag = 0;
			change_step_flag = 1;
			sub_current_step = 1;
			sub_change_step_flag = 1;
			current_temp = 0;
			s2_s3_start_flag = 0;
			l2_mid_start_check = 0;
			l3_full_start_check = 0;
			while (!end_flag && program_selected_flag && !error_flag) // end its self or stop button raise  this flag&//start button pressed
			{
				// start any program with thin lines
				////////////////////////////any_program////////////////////////////
				// if(!program_selected_flag||error_flag||end_flag)break;
				debuge_flag = 1;
				PORTC |= (1 << door_lock); // close door
				display_details();
				handle_errors();
				if (step_flag && current_step < total_steps) // increment step after step finished and raise its flag(stage flag)
				{											 // auto increment step
					current_step++;
					step_flag = 0;
					// it is used to call start_timer0 one time
					// start button set it
					change_step_flag = 1;
				}
				////////////////////////////any_program////////////////////////////

				else if (current_step == 1)
				{
					if (stage_flag && current_step < total_steps) // increment step after step finished and raise its flag(stage flag)
					{
						current_step++;
						stage_flag = 0;
						// it is used to call start_timer0 one time
						// start button set it
						change_step_flag = 1;
					}
					else if (current_step == 1 && change_step_flag) // 400
					{
						// expected_time =87140;//29min > 1740 sec
						change_step_flag = 0;
						stage = 1;
						start_timer1(); // must called only one time
					}
					else if (current_step == 2 && change_step_flag) // 180
					{
						// expected_time =67107 ;//1340 sec
						change_step_flag = 0;
						stage = 2;
						start_timer1();
					}
					else if (current_step == 3 && change_step_flag)
					{
						// expected_time =58092 ;//1160 sec
						change_step_flag = false;
						stage = 1;
						start_timer1();
					}
					else if (current_step == 4 && change_step_flag)
					{
						// expected_time =38061 ;//760 sec
						change_step_flag = false;
						stage = 2;
						start_timer1();
					}
					else if (current_step == 5 && change_step_flag)
					{
						expected_time = 29046; // 580
						change_step_flag = 0;
						stage = 1;
						start_timer1();
					}
					else if (current_step == 6 && change_step_flag)
					{
						// expected_time =9014;//180 sec
						// if(expected_time <1)expected_time=0;
						change_step_flag = 0;
						stage = 2;
						start_timer1();
					}
					else if (current_step == 7 && change_step_flag)
					{
						// expected_time =9014;//180 sec
						// if(expected_time <1)expected_time=0;
						change_step_flag = 0;
						stage = 3; // spin
						start_timer1();
					}
				}

				////////////////////////finish_program_sequence//////////////////////
				/// these lines used in any program to finish it
				else if (step_flag && !change_step_flag && current_step == total_steps)
				{ // finish program
					step_flag = 0;
					end_flag = 1;				// cleared in start button
					program_selected_flag = 0;	// cleared with start button for waiting start button press again to start program
					PORTC &= ~(1 << door_lock); // open_door
					tones(0);					// make sound after finishing
					// finish program
					break;
				}
			}
		}
		if (program == 6) // prewash
		{
			// program details
			// display_details();
			// 6 steps>>>stage1,2 *3times end process when all steps finished
			needed_temp = 0;
			total_steps = 6;
			current_step = 1;
			step_flag = 0;
			change_step_flag = 1;
			sub_current_step = 1;
			sub_change_step_flag = 1;
			current_temp = 0;
			s2_s3_start_flag = 0;
			l2_mid_start_check = 0;
			l3_full_start_check = 0;
			while (!end_flag && program_selected_flag && !error_flag) // end its self or stop button raise  this flag&//start button pressed
			{
				// start any program with thin lines
				////////////////////////////any_program////////////////////////////
				// if(!program_selected_flag||error_flag||end_flag)break;
				debuge_flag = 1;
				PORTC |= (1 << door_lock); // close door
				display_details();
				handle_errors();
				if (step_flag && current_step < total_steps) // increment step after step finished and raise its flag(stage flag)
				{											 // auto increment step
					current_step++;
					step_flag = 0;
					// it is used to call start_timer0 one time
					// start button set it
					change_step_flag = 1;
				}
				////////////////////////////any_program////////////////////////////

				else if (current_step == 1) // 400
				{
					// Prewash();
				}
				else if (current_step == 2)
				{
					Wash();
				}

				////////////////////////finish_program_sequence//////////////////////
				/// these lines used in any program to finish it
				else if (step_flag && !change_step_flag && current_step == total_steps)
				{ // finish program
					step_flag = 0;
					end_flag = 1;				// cleared in start button
					program_selected_flag = 0;	// cleared with start button for waiting start button press again to start program
					PORTC &= ~(1 << door_lock); // open_door
					tones(0);					// make sound after finishing
					// finish program
					break;
				}
			}
		}
	} // while(1)
} // main



/*
		heating_counter++;
		PORTC&=~(1<<s1);
		if (needed_temp!=0)
		{
			if (heating_counter <= 500)
			{
				//con1,con2 last alternating until heating process determine what should happen

				loopA_stage1 = 4 ;//to need to stop it now unless 12min heating finished or 20 min heating errorr
				PORTC|=(1<<s2);
				PORTC|=(1<<s3);
			}
			else if (heating_counter > 500)//10sec
			{
				PORTC&=~(1<<s2);
				PORTC&=~(1<<s3);//turn off
				heating_process= 1;//start heating in program loop
			}
			else if ((heating_counter >= 36057 ) && (heating_counter <= 60095  ))//turn off after 12 min
			{
				if ((current_temp >= needed_temp+1)||(current_temp <= needed_temp-1))
				{
					heating_process = 0;//stop heater
					needed_temp = 0;
					loopA_stage1=4;
					loopB_stage1=1;//2round
					counter_1 = 0;
					heating_counter=0;
					TIMSK&=~(1<<OCIE1A);//stoptimer
				}

			}
			else if (heating_counter >= 60096)//20min
			{
				heating_process = 0;
				heat_error=1;
				heating_counter=0;
				TIMSK&=~(1<<OCIE1A);//stoptimer
			}

		}
		else heating_process = 0;
		*/
