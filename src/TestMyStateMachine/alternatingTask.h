#ifndef _ALTERNATING_H_
#define _ALTERNATING_H_

#include "API.h"
/*_CAN BE EDITED_*/
// all in seconds
#define ALTER_MOTOR_ROTATE_RIGHT_time 20
#define ALTER_MOTOR_ROTATE_LEFT_time 20
#define ALTER_MOTOR_ROTATE_STOP_time 5
#define ALTER_TOTAL_time 200 // multiple of 50 (20+5+20+5)

extern WASHING_MACHINE_ERRORS_e WATERLEVELFILLFULLError;
extern volatile float ProgramTimer_inSec; // 65,535//max here about 18 hour //max needed 60*60*24=86400// main timer in seconds ,starts when user press start program after choosing it used for alternating statemachine and other
volatile uint16_t ALTERNATING_TIME = 0;
extern volatile uint8_t ALTERNATING_START_POINT;
/*_CAN BE EDITED_*/

typedef enum
{ // WRITE IT IN ORGANIZED WAY (ORDER IT)
  // ALTER_MOTOR_ROTATE_RIGHT_state,
  // ALTER_MOTOR_STOP_state,
  // ALTER_MOTOR_ROTATE_LEFT_state,
  // ALTER_FILL_WATER_LEV1_state, // LEVEL 1
    ALTERNATING_RUNNING_state,
    ALTERNATING_STATE_FINISHED_state, // usually put it in last line which is used for finishing the state
    ALTERNATING_EVENT_FINISHED_state, // used to finish your event no function from array will be excuted
} ALTERNATING_STATEs_e;

typedef ALTERNATING_STATEs_e (*ALTERNATING_EVENTs_fptr)(uint16_t); // poiter to function

typedef struct _
{
    // ALTERNATING_STATEs_e ALTER_st;//not needed
    // ALTERNATING_EVENTs_e ALTER_ev;//not needed
    // uint8_t ALTER_ev_time; // time of every event in seconds
    ALTERNATING_EVENTs_fptr ALTER_handler; // pointer to function

} ALTERNATING_stateMachine_t;

// return next state you want to run
ALTERNATING_STATEs_e ALTER_MOTOR_ROTATE_RIGHT_event_handler(uint16_t event_time)
{
    /*only for debugging*/
    PRINTDEBUG_T("RIGHT_event\n")

    if (ProgramTimer_inSec >= event_time + ALTER_MOTOR_ROTATE_RIGHT_time)
    {
        // finished and go to next state which defined in the array
        ALTERNATING_TIME = ProgramTimer_inSec; // update the event timer

        /*Write here your code*/
        OFF(RM_PORT, RM_PIN);
        /*end here*/

        return ALTERNATING_STATE_FINISHED_state;
    }
    else
    {
        /*Write here your code*/
        ON(RM_PORT, RM_PIN); // 20,5,20,5,20,5
        /*end here*/
    }
    return ALTERNATING_RUNNING_state;
}

ALTERNATING_STATEs_e ALTER_MOTOR_STOP_event_handler(uint16_t event_time)
{
    PRINTDEBUG_T("STOP_event\n")
    if (ProgramTimer_inSec >= event_time + ALTER_MOTOR_ROTATE_STOP_time)
    {
        OFF(RM_PORT, RM_PIN);
        OFF(LM_PORT, LM_PIN);
        ALTERNATING_TIME = ProgramTimer_inSec; // update the event timer
        return ALTERNATING_STATE_FINISHED_state;
    }
    else
    {
        OFF(RM_PORT, RM_PIN);
        OFF(LM_PORT, LM_PIN);
    }
    return ALTERNATING_RUNNING_state;
}
ALTERNATING_STATEs_e ALTER_MOTOR_ROTATE_LEFT_event_handler(uint16_t event_time)
{
    PRINTDEBUG_T("LEFT_event\n")
    if (ProgramTimer_inSec >= event_time + ALTER_MOTOR_ROTATE_LEFT_time)
    {
        // after finishing the event
        ALTERNATING_TIME = ProgramTimer_inSec; // update the event timer
        OFF(LM_PORT, LM_PIN);
        return ALTERNATING_STATE_FINISHED_state;
    }
    else
        ON(LM_PORT, LM_PIN); // 20,5,20,5,20,5
    return ALTERNATING_RUNNING_state;
}

ALTERNATING_STATEs_e ALTER_FINISH_event_handler(uint16_t event_time)
{
    PRINTDEBUG_T("FINISH_event\n")
    ALTERNATING_TIME = ProgramTimer_inSec; // update the event timer
    return ALTERNATING_EVENT_FINISHED_state;
}

// you define state sequence
ALTERNATING_stateMachine_t ALTERNATING_stateMachine_SEQUENCE_a[] = {

    /*state1*/ {ALTER_MOTOR_ROTATE_RIGHT_event_handler}, // 0 index
    /*state2*/ {ALTER_MOTOR_STOP_event_handler},         // 1 index
    /*state3*/ {ALTER_MOTOR_ROTATE_LEFT_event_handler},  // 2 index
    /*state4*/ {ALTER_MOTOR_STOP_event_handler},         // 3 index
                                                         // IF NO LOOPING finish it
    /*state5*/ {ALTER_FINISH_event_handler},             // 4 index

};
//

// start state
ALTERNATING_STATEs_e ALTER_nextState = ALTERNATING_EVENT_FINISHED_state; // must call init before run
void INIT_ALTERNATING(void)
{
    PRINTDEBUG_T("AlternInit\n")
    OFF(RM_PORT, RM_PIN);
    OFF(LM_PORT, LM_PIN);
    ALTER_nextState = ALTERNATING_RUNNING_state;
    ALTERNATING_TIME = ProgramTimer_inSec;
}

void ALTERNATING_RUN(void) // AS A SUB_STATEMACHINE
{
    if (ALTER_nextState == ALTERNATING_EVENT_FINISHED_state)
        return; // do nothing
    /** only this part have to be changed if array of sequence edited (total time of every event will be exceuted)**/
    uint16_t OneLoopEventsTime = (ALTER_MOTOR_ROTATE_RIGHT_time + ALTER_MOTOR_ROTATE_LEFT_time + 2 * ALTER_MOTOR_ROTATE_STOP_time);
    if ((ALTER_TOTAL_time % OneLoopEventsTime))
    {
        PRINTDEBUG_V("Error : Total Time Must be Multiple of:", OneLoopEventsTime)
        return;
    }

    static uint8_t ALTERNATING_SEQUENCE_count = 0 ;
    static uint16_t residualTime = ALTER_TOTAL_time; // ex:200
    
    if(ALTERNATING_START_POINT){
        ALTERNATING_SEQUENCE_count = ALTERNATING_START_POINT;
        ALTERNATING_START_POINT = 0;
        }

    /*The Core*/
    ALTER_nextState = (*ALTERNATING_stateMachine_SEQUENCE_a[ALTERNATING_SEQUENCE_count].ALTER_handler)(ALTERNATING_TIME);

    if (ALTER_nextState == ALTERNATING_STATE_FINISHED_state)
    {
        ALTERNATING_SEQUENCE_count++; // swap to next state to be run
    }
    // RUN and check for if looping found

    if ((ALTER_nextState == ALTERNATING_EVENT_FINISHED_state) && (residualTime > OneLoopEventsTime))
    {
        PRINTDEBUG_T("NewLoop")
        ALTERNATING_SEQUENCE_count = ALTERNATING_START_POINT;              // reset it
        ALTER_nextState = ALTERNATING_RUNNING_state; // reset state
        if (residualTime > 0)
            residualTime -= OneLoopEventsTime;
    }
}

void ALTERNATING_END(void)
{
    ALTER_nextState = ALTERNATING_EVENT_FINISHED_state;
}

#endif