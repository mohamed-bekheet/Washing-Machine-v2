#ifndef _waterLevel_FILL_Task_H_
#define _waterLevel_FILL_Task_H_

#include "API.h"
#include "USER_BUTTONS.h"
#include "ERRORS.h"


// all in seconds

extern WASHING_MACHINE_ERRORS_e WATERLEVELFILLFULLError;
#define WATERLEVELFILLFULL_FILL_WATER_LEV1_START_afterTime 10
#define WATERLEVELFILLFULL_CHECK_FILL_AFTER_TIME 10 // 3MIN
#define WATERLEVELFILLFULL_TOTAL_time 20            // multiple of 19 (20+5+20+5) FOR AUTO LOOPING
extern volatile float ProgramTimer_inSec;           // 65,535//max here about 18 hour //max needed 60*60*24=86400// main timer in seconds ,starts when user press start program after choosing it used for WATERLEVELFILLFULL statemachine and other
volatile uint16_t WATERLEVELFILLFULL_TIME = 0;
extern volatile uint8_t WATERLEVELFILLFULL_START_POINT;
extern USER_BUTTONS_STATEs_e USER_BUTTONS_State ; // must call init before run

typedef enum
{
    WATERLEVELFILLFULL_PAUSED_state,
    WATERLEVELFILLFULL_RUNNING_state,
    WATERLEVELFILLFULL_STATE_FINISHED_state, // usually put it in last line which is used for finishing the state
    WATERLEVELFILLFULL_EVENT_FINISHED_state, // used to finish your event no function from array will be excuted
} WATERLEVELFILLFULL_STATEs_e;

typedef WATERLEVELFILLFULL_STATEs_e (*WATERLEVELFILLFULL_EVENTs_fptr)(uint16_t); // poiter to function

// return next state you want to run
WATERLEVELFILLFULL_STATEs_e WATERLEVELFILLFULL_START_FILLING_AFTER_event_handler(uint16_t event_time)
{
    /*only for debugging*/
    PRINTDEBUG_T("START_FILL_AFTER_event\n")

    if (ProgramTimer_inSec >= event_time + WATERLEVELFILLFULL_FILL_WATER_LEV1_START_afterTime)
    {
        // finished and go to next state which defined in the array
        WATERLEVELFILLFULL_TIME = ProgramTimer_inSec; // update the event timer

        /*Write here your code*/
        // start fill water
        // IF IS THERE WATER  RAISE ERROR
        if (!READ(WLV3_PORT, WLV3_PIN)) // IF WATERLEVEL NOT FULL
        {
            ON(WSOLINOID1_PORT, WSOLINOID1_PIN)
            WATERLEVELFILLFULLError = ERROR_WATER_FILL_NO_PROBLEM;
        }
        else
            WATERLEVELFILLFULLError = ERROR_WATER_FULL_WATER_STILL_IN_TANK; // MAY PROBLEM IN PUMP

        /*end here*/

        return WATERLEVELFILLFULL_STATE_FINISHED_state;
    }
    else
    { // normal
        /*Write here your code*/
        // do no thing
        /*end here*/
    }
    return WATERLEVELFILLFULL_RUNNING_state;
}

WATERLEVELFILLFULL_STATEs_e WATERLEVELFILLFULL_CHECK_FILLING_AFTER_event_handler(uint16_t event_time)
{
    /*_only for debugging_*/
    PRINTDEBUG_T("CHECK_FILL_AFTER_event\n")

    if (ProgramTimer_inSec >= event_time + WATERLEVELFILLFULL_CHECK_FILL_AFTER_TIME)
    {
        // finished and go to next state which defined in the array
        WATERLEVELFILLFULL_TIME = ProgramTimer_inSec; // update the event timer

        /*Write here your code*/
        // start fill water
        int x = READ(WLV2_PORT, WLV3_PIN);
        x = 1;
        if (x)
        { // IF FILLED (GOOD)
            PRINTDEBUG_T("WATER LEVEL_MAX\n")
            WaterLevelError = ERROR_WATER_FILL_NO_PROBLEM; // CHECK PRESSURE SENSOR
        }
        else
        { // IF NOT FILLED (PROBLEM)

            PRINTDEBUG_T("WATER_ERROR\n")
            WaterLevelError = ERROR_WATER_FULL_TANK_NOT_FILLED;
        }

        /*end here*/

        return WATERLEVELFILLFULL_STATE_FINISHED_state;
    }
    else
    { // normal
        /*Write here your code*/
        // do no thing
        /*end here*/
    }
    return WATERLEVELFILLFULL_RUNNING_state;
}

WATERLEVELFILLFULL_STATEs_e WATERLEVELFILLFULL_FINISH_event_handler(uint16_t event_time)
{
    PRINTDEBUG_T("FINISH_event\n")
    /*WRITE LAST STATE YOU WANT YOUR TASK STOP AT*/
    OFF(WSOLINOID1_PORT, WSOLINOID1_PIN)

    WATERLEVELFILLFULL_TIME = ProgramTimer_inSec; // update the event timer
    return WATERLEVELFILLFULL_EVENT_FINISHED_state;
}

// you define state sequence
WATERLEVELFILLFULL_EVENTs_fptr WATERLEVELFILLFULL_stateMachine_SEQUENCE_a[] = {

    /*state1*/ {WATERLEVELFILLFULL_START_FILLING_AFTER_event_handler}, // 0 index
    /*state2*/ {WATERLEVELFILLFULL_CHECK_FILLING_AFTER_event_handler}, // 1 index

    // IF NO LOOPING finish it
    /*state5*/ {WATERLEVELFILLFULL_FINISH_event_handler}, // 4 index

};
//

// start state
WATERLEVELFILLFULL_STATEs_e WATERLEVELFILLFULL_nextState = WATERLEVELFILLFULL_EVENT_FINISHED_state; // must call init before run
void INIT_WATERLEVELFILLFULL(void)
{
    PRINTDEBUG_T("WATERLEVELFILLFULLnInit\n")
    WATERLEVELFILLFULL_nextState = WATERLEVELFILLFULL_RUNNING_state;
    WATERLEVELFILLFULL_TIME = ProgramTimer_inSec;
}
void WATERLEVELFILLFULL_END(void)
{
    WATERLEVELFILLFULL_FINISH_event_handler(1);//1 NOT USED
}

void WATERLEVELFILLFULL_ERROR_HANDLER(void)
{
    switch (WaterLevelError)
    {
    case ERROR_WATER_FULL_WATER_STILL_IN_TANK:
        {WATERLEVELFILLFULL_END();}
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
void WATERLEVELFILLFULL_LESTIN_TO_USER_BUTTONS(char MODESELECTOR){
    switch (MODESELECTOR)
    {
    case 'N'://NO ERROR NORMAL MODE
        switch (USER_BUTTONS_CHECK_PRESSING_RUN())
    {
    case USER_BUTTONS_PAUSED_PRESSED_state:
        {WATERLEVELFILLFULL_END();
        break;}
    case USER_BUTTONS_START_PRESSED_state:
        {/* code */
        INIT_WATERLEVELFILLFULL();
        break;}
    case USER_BUTTONS_STOP_PRESSED_state:
        {/* code */
        WATERLEVELFILLFULL_END();
        break;}
    case USER_BUTTONS_UP_PRESSED_state:
        {/* code */
        break;}
    case USER_BUTTONS_NOT_PRESSED_state:
        {/* code */
        PRINTDEBUG_T("FROM WATERLEVEL NO BUTTTON PRESED")
        break;}    
    }
    
        break;
    case 'E'://ERROR
    default:
        break;
    }
}

// MODE SELECTOR USED TO SPECIFY FILLING WATER OR WATER OUT(DRAIN)
void WATERLEVELFILLFULL_FILLING_RUN() // AS A SUB_STATEMACHINE
{
    if (WATERLEVELFILLFULL_nextState == WATERLEVELFILLFULL_EVENT_FINISHED_state)
    {
        return; // do nothing
    }

    if (WATERLEVELFILLFULL_nextState == WATERLEVELFILLFULL_PAUSED_state)
    {
        return;
    }
    if (WaterLevelError != ERROR_WATER_FILL_NO_PROBLEM)
    {
        WATERLEVELFILLFULL_LESTIN_TO_USER_BUTTONS('E');
    }
    

    /** only this part have to be changed if array of sequence edited (total time of every event will be exceuted)**/
    uint16_t OneLoopEventsTime = (WATERLEVELFILLFULL_CHECK_FILL_AFTER_TIME + WATERLEVELFILLFULL_FILL_WATER_LEV1_START_afterTime);
    // CHECK FOR VALID TOTAL TIME
    if ((WATERLEVELFILLFULL_TOTAL_time % OneLoopEventsTime))
    {
        PRINTDEBUG_V("Total Time Error Must be Multiple of:", OneLoopEventsTime)
        return;
    }

    static uint8_t WATERLEVELFILLFULL_SEQUENCE_count = 0;         // USED TO SWAP TO NEXT STATE RELATED FUNCTION
    static uint16_t residualTime = WATERLEVELFILLFULL_TOTAL_time; // ex:200 RESIDUAL TIME FROM TOTAL EVENT TO HANDLE LOOPING TIMES

    // TO START FROM SPECIFIC STATE THEN RESET IT TO START FROM FIRST STATE IN THE NEXT CALL OF THE TASK
    if (WATERLEVELFILLFULL_START_POINT > 0)
    {
        WATERLEVELFILLFULL_SEQUENCE_count = WATERLEVELFILLFULL_START_POINT;
        WATERLEVELFILLFULL_START_POINT = 0; // RESET IT TO FIRST FUNCTION
    }

    /*ERROR HANDLER AND TAKE DECICIONS*/
    WATERLEVELFILLFULL_ERROR_HANDLER();
    
    /*The Core*/
    WATERLEVELFILLFULL_nextState = (*WATERLEVELFILLFULL_stateMachine_SEQUENCE_a[WATERLEVELFILLFULL_SEQUENCE_count])(WATERLEVELFILLFULL_TIME);

    WATERLEVELFILLFULL_LESTIN_TO_USER_BUTTONS('N');

    if (WATERLEVELFILLFULL_nextState == WATERLEVELFILLFULL_STATE_FINISHED_state)
    {
        WATERLEVELFILLFULL_SEQUENCE_count++; // swap to next state to be run
    }

    // RUN and check for if looping found
    if ((WATERLEVELFILLFULL_nextState == WATERLEVELFILLFULL_EVENT_FINISHED_state) && (residualTime > OneLoopEventsTime))
    {
        PRINTDEBUG_T("NewLoop")
        WATERLEVELFILLFULL_SEQUENCE_count = WATERLEVELFILLFULL_START_POINT; // reset it
        WATERLEVELFILLFULL_nextState = WATERLEVELFILLFULL_RUNNING_state;    // reset state
        if (residualTime > 0)
            residualTime -= OneLoopEventsTime;
    }
}




#endif