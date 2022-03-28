#ifndef __API_C_H_
#define __API_C_H_

#define API_C 1 //write zero to deactivate
#define API_AVR 0
#define API_ESP32 0
#define API_ESP8266 0
#define API_ESP32 0

#if API_C
#define F_CPU 8000000
#define PORTA 0xFF
#define PA0 0
#define PA1 1
#define PA2 2
#define PA3 3
#define PA4 4
#define PA5 5
#define PA6 6
#define PA7 7

// ONPUT PINS
#define RM_PIN PA0     // RELAYS FOR SIGNAL TO INVERTER TO GET RIGHT ROTATE
#define LM_PIN PA1     // RELAYS FOR SIGNAL TO INVERTER TO GET LEFT ROTATE
#define SPEED1_PIN PA2 // RELAYS FOR SIGNAL TO INVERTER TO GET SPEED1
#define SPEED2_PIN PA3
#define SPEED3_PIN PA4 // motor speed 5
#define HEATER_PIN PA5 // Heater RELAY
//OUTPUT PORTS
#define RM_PORT PORTA     // RELAYS FOR SIGNAL TO INVERTER TO GET RIGHT ROTATE
#define LM_PORT PORTA     // RELAYS FOR SIGNAL TO INVERTER TO GET LEFT ROTATE
#define SPEED1_PORT PORTA // RELAYS FOR SIGNAL TO INVERTER TO GET SPEED1
#define SPEED2_PORT PORTA
#define SPEED3_PORT PORTA // motor speed 3
#define HEATER_PORT PORTA // Heater RELAY

#define WSOLINOID1_PIN PA0     // RELAYS FOR SIGNAL TO SOLINOID TO FILL WATER
#define WSOLINOID1_PORT PORTA     // RELAYS FOR SIGNAL TO SOLINOID TO FILL WATER
#define WSOLINOID2_PIN PA0     // RELAYS FOR SIGNAL TO SOLINOID TO FILL WATER
#define WSOLINOID2_PORT PORTA     // RELAYS FOR SIGNAL TO SOLINOID TO FILL WATER
#define WSOLINOID3_PIN PA0     // RELAYS FOR SIGNAL TO SOLINOID TO FILL WATER
#define WSOLINOID3_PORT PORTA     // RELAYS FOR SIGNAL TO SOLINOID TO FILL WATER
#define WPUMP_PIN PA0     // RELAYS FOR SIGNAL TO WATER PUMP TO CGET WATER OUT
#define WPUMP_PORT PORTA     // RELAYS FOR SIGNAL TO WATER PUMP TO GET WATER OUT
#define DSOLINOID3_PIN PA0     // RELAYS FOR SIGNAL TO SOLINOID TO CLOSE DOOR
#define DSOLINOID3_PORT PORTA     // RELAYS FOR SIGNAL TO SOLINOID TO CLOSE DOOR


// INPUT PINS AND PORTS
#define WLV1_PIN PA1   // PRESSURE SENSOR FOR WATER LEVEL1 (MIN LEVEL)
#define WLV2_PIN PA2   // PRESSURE SENSOR FOR WATER LEVEL2 (MID LEVEL)
#define WLV3_PIN PA3   // PRESSURE SENSOR FOR WATER LEVEL3 (MAX LEVEL)
#define WLV1_PORT PA1   // PRESSURE SENSOR FOR WATER LEVEL1 (MIN LEVEL)
#define WLV2_PORT PA2   // PRESSURE SENSOR FOR WATER LEVEL2 (MID LEVEL)
#define WLV3_PORT PA3   // PRESSURE SENSOR FOR WATER LEVEL3 (MAX LEVEL)

#define SPEED3_PIN PA4 // motor speed 5
#define HEATER_PIN PA5 // Heater RELAY


#define LM_PORT PORTA     // RELAYS FOR SIGNAL TO INVERTER TO GET LEFT ROTATE
#define SPEED1_PORT PORTA // RELAYS FOR SIGNAL TO INVERTER TO GET SPEED1
#define SPEED2_PORT PORTA
#define SPEED3_PORT PORTA // motor speed 5
#define HEATER_PORT PORTA // Heater RELAY


//BUTTONS
#define PAUSE_PIN  PORTA
#define PAUSE_PORT PORTA
#define START_PIN  PORTA
#define START_PORT PORTA
#define STOP_PIN   PORTA
#define STOP_PORT  PORTA
#define UP_PIN     PORTA
#define UP_PORT    PORTA
#define DOWN_PIN   PORTA
#define DOWN_PORT  PORTA

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define setbit(port, bit) (port |= (1 << bit))
#define clrbit(port, bit) (port &= ~(1 << bit))
#define PRINTDEBUG_T(Text) printf(Text"\n");
#define PRINTDEBUG_V(Text,Value) printf(Text"%d\n",Value);
#define PRINTDEBUG_Vf(Text,Value) printf(Text"%f\n",Value);

#define ACTIVE_HIGH
#ifdef ACTIVE_LOW
#define OFF(port, bit) (port |= (1 << bit))
#define ON(port, bit) (port &= ~(1 << bit))
#endif
#ifdef ACTIVE_HIGH
#define ON(port, bit) printf("ON_Port:%x,Bit:%d\n", port, bit);
#define OFF(port, bit) printf("OFF_Port:%x,Bit:%d\n", port, bit);
#endif
#define READ(REG, pin) getchar()

#include <time.h>

void delay(int milliseconds)
{
    long pause;
    clock_t now, then;

    pause = milliseconds * (CLOCKS_PER_SEC / 1000);
    now = then = clock();
    while ((now - then) < pause)
        now = clock();
}



#endif

#if API_AVR
#define F_CPU 8000000
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <stdlib.h>

// INPUT PINS
#define RM_PIN PA0     // RELAYS FOR SIGNAL TO INVERTER TO GET RIGHT ROTATE
#define LM_PIN PA1     // RELAYS FOR SIGNAL TO INVERTER TO GET LEFT ROTATE
#define SPEED1_PIN PA2 // RELAYS FOR SIGNAL TO INVERTER TO GET SPEED1
#define SPEED2_PIN PA3
#define SPEED3_PIN PA4 // motor speed 5
#define HEATER_PIN PA5 // Heater RELAY

#define RM_PORT PORTA     // RELAYS FOR SIGNAL TO INVERTER TO GET RIGHT ROTATE
#define LM_PORT PORTA     // RELAYS FOR SIGNAL TO INVERTER TO GET LEFT ROTATE
#define SPEED1_PORT PORTA // RELAYS FOR SIGNAL TO INVERTER TO GET SPEED1
#define SPEED2_PORT PORTA
#define SPEED3_PORT PORTA // motor speed 5
#define HEATER_PORT PORTA // Heater RELAY

//BUTTONS
#define PAUSE_PIN  PD3 
#define PAUSE_PORT PIND
#define START_PIN  PB4 
#define START_PORT PINB
#define STOP_PIN   PD2 
#define STOP_PORT  PIND
#define UP_PIN     PD0 
#define UP_PORT    PINA
#define DOWN_PIN   PD1 
#define DOWN_PORT  PIND

#define ACTIVE_HIGH
#ifdef ACTIVE_LOW
#define OFF(port, bit) (port |= (1 << bit))
#define ON(port, bit) (port &= ~(1 << bit))
#endif
#ifdef ACTIVE_HIGH
#define ON(OUTport, bit) (OUTport |= (1 << bit))
#define OFF(OUTport, bit) (OUTport &= ~(1 << bit))
#endif
#endif
#define Read(INport, pin) (~INPINB) & (1 << pin)
#endif