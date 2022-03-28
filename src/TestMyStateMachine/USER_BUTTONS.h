#ifndef _USER_BUTTONS_H_
#define _USER_BUTTONS_H_

#include "API.h"
#include "ERRORS.h"

// all in seconds

#define USER_BUTTONS_FILL_WATER_LEV1_START_afterTime 10
#define USER_BUTTONS_CHECK_FILL_AFTER_TIME 10 // 3MIN
#define USER_BUTTONS_TOTAL_time 20            // multiple of 19 (20+5+20+5) FOR AUTO LOOPING

extern WASHING_MACHINE_ERRORS_e USER_BUTTONSError;
extern volatile float ProgramTimer_inSec;           // 65,535//max here about 18 hour //max needed 60*60*24=86400// main timer in seconds ,starts when user press start program after choosing it used for USER_BUTTONS statemachine and other
volatile uint16_t USER_BUTTONS_TIME = 0;
extern volatile uint8_t USER_BUTTONS_START_POINT;

 typedef enum
{
    
    USER_BUTTONS_PAUSED_PRESSED_state,
    USER_BUTTONS_START_PRESSED_state,
    USER_BUTTONS_STOP_PRESSED_state,
    USER_BUTTONS_UP_PRESSED_state,
    USER_BUTTONS_DOWN_PRESSED_state,

    USER_BUTTONS_NOT_PRESSED_state,

    USER_BUTTONS_PAUSED_state,
    USER_BUTTONS_RUNNING_state,
    USER_BUTTONS_STATE_FINISHED_state, // usually put it in last line which is used for finishing the state
    USER_BUTTONS_EVENT_FINISHED_state, // used to finish your event no function from array will be excuted
} USER_BUTTONS_STATEs_e;



// start state
extern USER_BUTTONS_STATEs_e USER_BUTTONS_State ; // must call init before run

void INIT_USER_BUTTONS(void)
{
    PRINTDEBUG_T("USER_BUTTONSnInit\n")
    USER_BUTTONS_State = USER_BUTTONS_RUNNING_state;
    USER_BUTTONS_TIME = ProgramTimer_inSec;
}


// MODE SELECTOR USED TO SPECIFY FILLING WATER OR WATER OUT(DRAIN)
USER_BUTTONS_STATEs_e USER_BUTTONS_CHECK_PRESSING_RUN() // AS A SUB_STATEMACHINE
{
        return USER_BUTTONS_NOT_PRESSED_state;
}

void USER_BUTTONS_END(void)
{/*NO CHANGE NEEDED HERE*/
    //USER_BUTTONS_FINISH_event_handler(1);//1 NOT USED
}

#endif



