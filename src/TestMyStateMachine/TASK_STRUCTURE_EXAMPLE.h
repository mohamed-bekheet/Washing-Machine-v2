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
    USER_BUTTONS_PAUSED_state,
    USER_BUTTONS_RUNNING_state,
    USER_BUTTONS_STATE_FINISHED_state, // usually put it in last line which is used for finishing the state
    USER_BUTTONS_EVENT_FINISHED_state, // used to finish your event no function from array will be excuted
} USER_BUTTONS_STATEs_e;

typedef USER_BUTTONS_STATEs_e (*USER_BUTTONS_EVENTs_fptr)(uint16_t); // poiter to function

// return next state you want to run
USER_BUTTONS_STATEs_e USER_BUTTONS_START_FILLING_AFTER_event_handler(uint16_t event_time)
{
    /*only for debugging*/
    PRINTDEBUG_T("START_FILL_AFTER_event\n")

    if (ProgramTimer_inSec >= event_time + USER_BUTTONS_FILL_WATER_LEV1_START_afterTime)
    {
        // finished and go to next state which defined in the array
        USER_BUTTONS_TIME = ProgramTimer_inSec; // update the event timer

        /*Write here your code*/



        /*end here*/

        return USER_BUTTONS_STATE_FINISHED_state;
    }
    else
    { // normal
        /*Write here your code*/

        // do no thing

        /*end here*/
    }
    return USER_BUTTONS_RUNNING_state;
}

USER_BUTTONS_STATEs_e USER_BUTTONS_FINISH_event_handler(uint16_t event_time)
{
    PRINTDEBUG_T("FINISH_event\n")
    /*WRITE LAST STATE YOU WANT YOUR TASK STOP AT*/
/*START WRITING HERE*/
    OFF(WSOLINOID1_PORT, WSOLINOID1_PIN)
/*END WRITING HERE*/

    USER_BUTTONS_TIME = ProgramTimer_inSec; // update the event timer
    return USER_BUTTONS_EVENT_FINISHED_state;
}

// you define state sequence
USER_BUTTONS_EVENTs_fptr USER_BUTTONS_stateMachine_SEQUENCE_a[] = {

    /*state1*/ {USER_BUTTONS_START_FILLING_AFTER_event_handler}, // 0 index
    /*state2*/ {USER_BUTTONS_START_FILLING_AFTER_event_handler}, // 1 index

    // IF NO LOOPING finish it
    /*state5*/ {USER_BUTTONS_FINISH_event_handler}, // 4 index

};
//

// start state
USER_BUTTONS_STATEs_e USER_BUTTONS_nextState = USER_BUTTONS_EVENT_FINISHED_state; // must call init before run
void INIT_USER_BUTTONS(void)
{
    PRINTDEBUG_T("USER_BUTTONSnInit\n")
    USER_BUTTONS_nextState = USER_BUTTONS_RUNNING_state;
    USER_BUTTONS_TIME = ProgramTimer_inSec;
}

void USER_BUTTONS_ERROR_HANDLER(void)
{
    /*CHANGE THOSE AS FOR YOUR REQUIRMENT*/
    switch (WaterLevelError)
    {
    case ERROR_WATER_FULL_WATER_STILL_IN_TANK:
        {USER_BUTTONS_END();}
        break;
    case ERROR_WATER_FULL_TANK_NOT_FILLED:
        {/* code */}
        break;
    case ERROR_WATER_FULL_FILLED:
        {/* code */}
        break;
    case ERROR_WATER_FILL_NO_PROBLEM:
        {/* code */}
        break;
    }
}
// MODE SELECTOR USED TO SPECIFY FILLING WATER OR WATER OUT(DRAIN)
void USER_BUTTONS_FILLING_RUN() // AS A SUB_STATEMACHINE
{
    /*NO CHANGE NEEDED HERE*/
    if (USER_BUTTONS_nextState == USER_BUTTONS_EVENT_FINISHED_state)
    {
        return; // do nothing
    }

    if (USER_BUTTONS_nextState == USER_BUTTONS_PAUSED_state)
    {
        return;
    }

    /** only this part have to be changed if array of sequence edited (total time of every event will be exceuted)**/
    uint16_t OneLoopEventsTime = (USER_BUTTONS_CHECK_FILL_AFTER_TIME + USER_BUTTONS_FILL_WATER_LEV1_START_afterTime);
    // CHECK FOR VALID TOTAL TIME
    if ((USER_BUTTONS_TOTAL_time % OneLoopEventsTime))
    {
        PRINTDEBUG_V("Total Time Error Must be Multiple of:", OneLoopEventsTime)
        return;
    }

    static uint8_t USER_BUTTONS_SEQUENCE_count = 0;         // USED TO SWAP TO NEXT STATE RELATED FUNCTION
    static uint16_t residualTime = USER_BUTTONS_TOTAL_time; // ex:200 RESIDUAL TIME FROM TOTAL EVENT TO HANDLE LOOPING TIMES

    // TO START FROM SPECIFIC STATE THEN RESET IT TO START FROM FIRST STATE IN THE NEXT CALL OF THE TASK
    if (USER_BUTTONS_START_POINT > 0)
    {
        USER_BUTTONS_SEQUENCE_count = USER_BUTTONS_START_POINT;
        USER_BUTTONS_START_POINT = 0; // RESET IT TO FIRST FUNCTION
    }

    /*ERROR HANDLER AND TAKE DECICIONS*/
    USER_BUTTONS_ERROR_HANDLER();
    /*The Core*/
    USER_BUTTONS_nextState = (*USER_BUTTONS_stateMachine_SEQUENCE_a[USER_BUTTONS_SEQUENCE_count])(USER_BUTTONS_TIME);

    if (USER_BUTTONS_nextState == USER_BUTTONS_STATE_FINISHED_state)
    {
        USER_BUTTONS_SEQUENCE_count++; // swap to next state to be run
    }

    // RUN and check for if looping found
    if ((USER_BUTTONS_nextState == USER_BUTTONS_EVENT_FINISHED_state) && (residualTime > OneLoopEventsTime))
    {
        PRINTDEBUG_T("NewLoop")
        USER_BUTTONS_SEQUENCE_count = USER_BUTTONS_START_POINT; // reset it
        USER_BUTTONS_nextState = USER_BUTTONS_RUNNING_state;    // reset state
        if (residualTime > 0)
            residualTime -= OneLoopEventsTime;
    }
}

void USER_BUTTONS_END(void)
{/*NO CHANGE NEEDED HERE*/
    USER_BUTTONS_FINISH_event_handler(1);//1 NOT USED
}

#endif



