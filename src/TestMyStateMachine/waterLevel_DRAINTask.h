#ifndef _waterLevel_DRAIN_Task_H_
#define _waterLevel_DRAIN_Task_H_

#include "API.h"
#include "ERRORS.h"

// all in seconds

extern WASHING_MACHINE_ERRORS_e WATERLEVELDRAINError;
#define WATERLEVELDRAIN_FILL_WATER_LEV1_START_afterTime 10
#define WATERLEVELDRAIN_CHECK_FILL_AFTER_TIME 10 // 3MIN
#define WATERLEVELDRAIN_TOTAL_time 20            // multiple of 19 (20+5+20+5) FOR AUTO LOOPING
extern volatile float ProgramTimer_inSec;       // 65,535//max here about 18 hour //max needed 60*60*24=86400// main timer in seconds ,starts when user press start program after choosing it used for WATERLEVELDRAIN statemachine and other
volatile uint16_t WATERLEVELDRAIN_TIME = 0;
extern volatile uint8_t WATERLEVELDRAIN_START_POINT;

typedef enum
{
    WATERLEVELDRAIN_RUNNING_state,
    WATERLEVELDRAIN_STATE_FINISHED_state, // usually put it in last line which is used for finishing the state
    WATERLEVELDRAIN_EVENT_FINISHED_state, // used to finish your event no function from array will be excuted
} WATERLEVELDRAIN_STATEs_e;

typedef WATERLEVELDRAIN_STATEs_e (*WATERLEVELDRAIN_EVENTs_fptr)(uint16_t); // poiter to function

// return next state you want to run
WATERLEVELDRAIN_STATEs_e WATERLEVELDRAIN_START_FILLING_AFTER_event_handler(uint16_t event_time)
{
    /*only for debugging*/
    PRINTDEBUG_T("START_FILL_AFTER_event\n")

    if (ProgramTimer_inSec >= event_time + WATERLEVELDRAIN_FILL_WATER_LEV1_START_afterTime)
    {
        // finished and go to next state which defined in the array
        WATERLEVELDRAIN_TIME = ProgramTimer_inSec; // update the event timer

        /*Write here your code*/
        // start fill water
        //IF IS THERE WATER  RAISE ERROR
        if (!READ(WLV3_PORT,WLV3_PIN))//IF WATERLEVEL NOT FULL
        {
            ON(WSOLINOID1_PORT, WSOLINOID1_PIN)
        }
        else WATERLEVELDRAINError = ERROR_WATER_FULL_WATER_STILL_IN_TANK;//MAY PROBLEM IN PUMP
        
        
        /*end here*/

        return WATERLEVELDRAIN_STATE_FINISHED_state;
    }
    else
    { // normal
        /*Write here your code*/
        // do no thing
        /*end here*/
    }
    return WATERLEVELDRAIN_RUNNING_state;
}

WATERLEVELDRAIN_STATEs_e WATERLEVELDRAIN_CHECK_FILLING_AFTER_event_handler(uint16_t event_time)
{
    /*_only for debugging_*/
    PRINTDEBUG_T("CHECK_FILL_AFTER_event\n")

    if (ProgramTimer_inSec >= event_time + WATERLEVELDRAIN_CHECK_FILL_AFTER_TIME)
    {
        // finished and go to next state which defined in the array
        WATERLEVELDRAIN_TIME = ProgramTimer_inSec; // update the event timer

        /*Write here your code*/
        // start fill water
        int x = READ(WLV2_PORT, WLV3_PIN);
        x = 1;
        if (x)
        {
            PRINTDEBUG_T("WATER_ERROR\n")
            WaterLevelError = ERROR_WATER_FULL_PROBLEM;
        }
        else
        {
            PRINTDEBUG_T("WATER LEVEL_MAX\n")
            WaterLevelError = ERROR_WATER_FULL_FILLED;
        }

        /*end here*/

        return WATERLEVELDRAIN_STATE_FINISHED_state;
    }
    else
    { // normal
        /*Write here your code*/
        // do no thing
        /*end here*/
    }
    return WATERLEVELDRAIN_RUNNING_state;
}
WATERLEVELDRAIN_STATEs_e WATERLEVELDRAIN_FINISH_event_handler(uint16_t event_time)
{
    PRINTDEBUG_T("FINISH_event\n")
    WATERLEVELDRAIN_TIME = ProgramTimer_inSec; // update the event timer
    return WATERLEVELDRAIN_EVENT_FINISHED_state;
}

// you define state sequence
WATERLEVELDRAIN_EVENTs_fptr WATERLEVELDRAIN_stateMachine_SEQUENCE_a[] = {

    /*state1*/ {WATERLEVELDRAIN_START_FILLING_AFTER_event_handler}, // 0 index
    /*state2*/ {WATERLEVELDRAIN_CHECK_FILLING_AFTER_event_handler}, // 1 index

    // IF NO LOOPING finish it
    /*state5*/ {WATERLEVELDRAIN_FINISH_event_handler}, // 4 index

};
//

// start state
WATERLEVELDRAIN_STATEs_e WATERLEVELDRAIN_nextState = WATERLEVELDRAIN_EVENT_FINISHED_state; // must call init before run
void INIT_WATERLEVELDRAIN(void)
{
    PRINTDEBUG_T("WATERLEVELDRAINnInit\n")
    WATERLEVELDRAIN_nextState = WATERLEVELDRAIN_RUNNING_state;
    WATERLEVELDRAIN_TIME = ProgramTimer_inSec;
}
//MODE SELECTOR USED TO SPECIFY FILLING WATER OR WATER OUT(DRAIN)
void WATERLEVELDRAIN_FILLING_RUN(uint8_t MODESELECTOR) // AS A SUB_STATEMACHINE
{
    if (WATERLEVELDRAIN_nextState == WATERLEVELDRAIN_EVENT_FINISHED_state)
        return; // do nothing
    /** only this part have to be changed if array of sequence edited (total time of every event will be exceuted)**/
    uint16_t OneLoopEventsTime = (WATERLEVELDRAIN_CHECK_FILL_AFTER_TIME + WATERLEVELDRAIN_FILL_WATER_LEV1_START_afterTime);
    if ((WATERLEVELDRAIN_TOTAL_time % OneLoopEventsTime))
    {
        PRINTDEBUG_V("Total Time Error Must be Multiple of:", OneLoopEventsTime)
        return;
    }

    static uint8_t WATERLEVELDRAIN_SEQUENCE_count = 0;
    static uint16_t residualTime = WATERLEVELDRAIN_TOTAL_time; // ex:200

        if(WATERLEVELDRAIN_START_POINT){
        WATERLEVELDRAIN_SEQUENCE_count = WATERLEVELDRAIN_START_POINT;
        WATERLEVELDRAIN_START_POINT = 0;
        }

    /*The Core*/
    WATERLEVELDRAIN_nextState = (*WATERLEVELDRAIN_stateMachine_SEQUENCE_a[WATERLEVELDRAIN_SEQUENCE_count])(WATERLEVELDRAIN_TIME);

    if (WATERLEVELDRAIN_nextState == WATERLEVELDRAIN_STATE_FINISHED_state)
    {
        WATERLEVELDRAIN_SEQUENCE_count++; // swap to next state to be run
    }
    // RUN and check for if looping found

    if ((WATERLEVELDRAIN_nextState == WATERLEVELDRAIN_EVENT_FINISHED_state) && (residualTime > OneLoopEventsTime))
    {
        PRINTDEBUG_T("NewLoop")
        WATERLEVELDRAIN_SEQUENCE_count = WATERLEVELDRAIN_START_POINT;                       // reset it
        WATERLEVELDRAIN_nextState = WATERLEVELDRAIN_RUNNING_state; // reset state
        if (residualTime > 0)
            residualTime -= OneLoopEventsTime;
    }
}

void WATERLEVELDRAIN_END(void)
{
    WATERLEVELDRAIN_nextState = WATERLEVELDRAIN_EVENT_FINISHED_state;
}

#endif