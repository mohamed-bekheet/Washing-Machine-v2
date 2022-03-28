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

// Try to apply state machine concept
// Washing machine have diffrent program every program have some of main subprograms(wash,rinse,prewash)
// every subprogram has parameters dependent on every program (as:Time,Times,sequence,)
// you have user input choose buttons should be carious about it all time

// Try in this version to be dynamically in your code
// If any edits needed just edit in main structure so  put it in your mind mohamed
/**
 * @brief STATE MACHINE DESIGN
 * CONSIST OF ENUM FOR ALL STATES AND EVENT
 * 2-STRUCT AND ITS POINTER TO FUNCTION
 *
 */
#define setbit(port, bit) (port |= (1 << bit))
#define clrbit(port, bit) (port &= ~(1 << bit))

#define ACTIVE_HIGH
#ifdef ACTIVE_LOW
#define OFF(port, bit) (port |= (1 << bit))
#define ON(port, bit) (port &= ~(1 << bit))
#endif

#ifdef ACTIVE_HIGH
#define ON(port, bit) (port |= (1 << bit))
#define OFF(port, bit) (port &= ~(1 << bit))
#endif
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

volatile uint16_t ProgramTimer_inSec = 0; // main timer in seconds ,starts when user press start program after choosing it used for alternating statemachine and other

// all in seconds
#define ALTER_MOTOR_ROTATE_RIGHT_time 20
#define ALTER_MOTOR_ROTATE_LEFT_time 20
#define ALTER_MOTOR_ROTATE_STOP_time 5
#define ALTER_FILL_WATER_LEV1_START_afterTime
#define ALTER_FILL_WATER_LEV1_ERROR_afterTime

typedef enum
{
    ALTER_MOTOR_ROTATE_RIGHT_state,
    ALTER_MOTOR_ROTATE_LEFT_state,
    ALTER_MOTOR_STOP_state,
    ALTER_FILL_WATER_LEV1_state, // LEVEL 1

    ALTERNATING_FINISHED_state // usually put it in last line which is used for finishing the state
} ALTERNATING_STATEs_e;

typedef enum
{
    ALTER_MOTOR_ROTATE_RIGHT_event,
    ALTER_MOTOR_ROTATE_LEFT_event,
    ALTER_MOTOR_STOP_event,
    ALTER_FILL_WATER_LEV1_event, // LEVEL 1
    ALTERNATING_FINISHED_event   // usually put it in last line which is used for finishing the event
} ALTERNATING_EVENTs_e;

typedef ALTERNATING_STATEs_e (*ALTERNATING_EVENTs_fptr)(uint8_t);

typedef struct
{
    ALTERNATING_STATEs_e ALTER_st;
    ALTERNATING_EVENTs_e ALTER_ev;
    uint8_t ALTER_ev_time; // time of every event in seconds
    ALTERNATING_EVENTs_fptr ALTER_handler;

} ALTERNATING_stateMachine_t;

// return next state you want to run
ALTERNATING_STATEs_e ALTER_MOTOR_ROTATE_RIGHT_event_handler(uint16_t event_time)
{
    ON(RM_PORT, RM_PIN); // 20,5,20,5,20,5
    if (ProgramTimer_inSec > event_time + ALTER_MOTOR_ROTATE_RIGHT_time)
    {
        OFF(RM_PORT, RM_PIN);
        return ALTER_MOTOR_ROTATE_LEFT_state;
    }
    return ALTER_MOTOR_STOP_state;
}
ALTERNATING_STATEs_e ALTER_MOTOR_ROTATE_LEFT_event_handler(void)
{

    return ALTER_MOTOR_ROTATE_RIGHT_state;
}
ALTERNATING_STATEs_e ALTER_MOTOR_STOP_event_handler(void)
{

    return ALTER_MOTOR_ROTATE_RIGHT_state;
}
ALTERNATING_STATEs_e ALTER_FILL_WATER_LEV1_event_handler(void)
{

    return ALTER_MOTOR_ROTATE_RIGHT_state;
}

// define state sequence to be dynamic software could be edited easily
// array of struct
ALTERNATING_stateMachine_t ALTERNATING_stateMachine_a[] = {
    /*state1*/ {ALTER_MOTOR_ROTATE_RIGHT_state, ALTER_MOTOR_ROTATE_RIGHT_event, 20, ALTER_MOTOR_ROTATE_RIGHT_event_handler}, // 0 index
    /*state2*/ {ALTER_MOTOR_STOP_state, ALTER_MOTOR_STOP_event, 5, ALTER_MOTOR_STOP_event_handler},
    /*state3*/ {ALTER_MOTOR_ROTATE_LEFT_state, ALTER_MOTOR_ROTATE_LEFT_event, 20, ALTER_MOTOR_ROTATE_LEFT_event_handler},
    /*state4*/ {ALTER_FILL_WATER_LEV1_state, ALTER_FILL_WATER_LEV1_event, 180, ALTER_FILL_WATER_LEV1_event_handler},
    /*state5*/ {ALTER_FILL_WATER_LEV1_state, ALTER_FILL_WATER_LEV1_event, 20, ALTER_FILL_WATER_LEV1_event_handler},

};

//
// start state
extern ALTERNATING_STATEs_e ALTER_nextState = ALTER_MOTOR_ROTATE_RIGHT_state;
volatile uint16_t ALTERNATING_TIME = 0;

void INIT_ALTERNATING(void)
{
    ALTERNATING_TIME = ProgramTimer_inSec;
}
void ALTERNATING_RUN(void)
{
    // static uint16_t event_timer =  ProgramTime;//executed only one time//event timer will not change if called agian

    if (ALTER_nextState < ALTERNATING_FINISHED_state)
        // call related function to next state ,ask it for new transition
        // excecute next event when returned state changed
        ALTER_nextState = (*ALTERNATING_stateMachine_a[ALTER_nextState].ALTER_handler)(ALTERNATING_TIME);
}

/**
 * @brief SUBPROGRAMS CONSIST OF COMPINATION OF THOSE STAGES OR THIS ENUM SO MUST BE IMPLEMENTED IN PROFESSIONAL WAY (EX:STATE MACHINE DESIGN)
 *
 */
/*
typedef enum
{
    ALTERNATING_STATE,
    //ALTERNATING_RUNNING_STATE,
    //ALTERNATING_FINISHED_STATE,

    DRIAN_STATE,
    SPIN_STATE,
    FILLUP_STATE, // TO FILL WASHING MACHINE WITH WATER
} SUBPROGRAMS_CORES_STATES;

//EVENTS RELATES TO STATES IF RETURNERD
typedef enum
{
    ALTERNATING_EVENT,
    DRIAN_EVENT,
    SPIN_EVENT,
    FILLUP_EVENT, // TO FILL WASHING MACHINE WITH WATER
} SUBPROGRAMS_CORES_EVENTS;

typedef SUBPROGRAMS_CORES_STATES (*SUBPROGRAMS_CORES_EVENTS_fptr)(void);//POINTER TO FUNCTION RETURN SUBPROGRAMS_CORES_STATES

//SUBPOROGRAMS_CORE
typedef struct{
    SUBPROGRAMS_CORES_STATES;
    SUBPROGRAMS_CORES_EVENTS;
    SUBPROGRAMS_CORES_EVENTS_fptr;

}SUBPROGRAMS_CORES_t;








*/

typedef enum
{
    PREWASH,
    WASH,
    RINSE,

} USER_SUBPROGRAMS;

typedef enum
{
    COLD,
    BLANKETS,
    POOLTOWELS,
    BLOOD,
} USER_PROGRAMS;
