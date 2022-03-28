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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
#define ON(port, bit) printf("ON_Port:%x,Bit:%d\n", port, bit);
#define OFF(port, bit) printf("OFF_Port:%x,Bit:%d\n", port, bit);
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

volatile uint16_t ALTERNATING_TIME = 0;

typedef enum
{   //WRITE IT IN ORGANIZED WAY (ORDER IT)
    ALTER_MOTOR_ROTATE_RIGHT_state,
    ALTER_MOTOR_STOP_state,
    ALTER_MOTOR_ROTATE_LEFT_state,
    //ALTER_FILL_WATER_LEV1_state, // LEVEL 1

    ALTERNATING_FINISHED_state // usually put it in last line which is used for finishing the state
} ALTERNATING_STATEs_e;

typedef enum
{
    ALTER_MOTOR_ROTATE_RIGHT_event,
    ALTER_MOTOR_STOP_event,
    ALTER_MOTOR_ROTATE_LEFT_event,
    ALTER_FILL_WATER_LEV1_event, // LEVEL 1
    ALTERNATING_FINISHED_event   // usually put it in last line which is used for finishing the event
} ALTERNATING_EVENTs_e;

typedef ALTERNATING_STATEs_e (*ALTERNATING_EVENTs_fptr)(uint16_t);

typedef struct
{
    ALTERNATING_STATEs_e ALTER_st;
    ALTERNATING_EVENTs_e ALTER_ev;
    //uint8_t ALTER_ev_time; // time of every event in seconds
    ALTERNATING_EVENTs_fptr ALTER_handler;

} ALTERNATING_stateMachine_t;

// return next state you want to run
ALTERNATING_STATEs_e ALTER_MOTOR_ROTATE_RIGHT_event_handler(uint16_t event_time)
{
    printf("RIGHT_event\n");
    if (ProgramTimer_inSec >= event_time + ALTER_MOTOR_ROTATE_RIGHT_time)
    {
        //finished abd go to next state
        ALTERNATING_TIME = ProgramTimer_inSec;//update the event timer
        OFF(RM_PORT, RM_PIN);
        return ALTER_MOTOR_STOP_state;
    }
    else
        ON(RM_PORT, RM_PIN); // 20,5,20,5,20,5
    return ALTER_MOTOR_ROTATE_RIGHT_state;
}
ALTERNATING_STATEs_e ALTER_MOTOR_STOP_event_handler(uint16_t event_time)
{
     printf("STOP_event\n");
     if (ProgramTimer_inSec >= event_time + ALTER_MOTOR_ROTATE_STOP_time){

        ALTERNATING_TIME = ProgramTimer_inSec;//update the event timer
        return ALTER_MOTOR_ROTATE_LEFT_state;
     }
     else {
         OFF(RM_PORT, RM_PIN);
         OFF(LM_PORT, LM_PIN);
     }
    return ALTER_MOTOR_STOP_state;
}
ALTERNATING_STATEs_e ALTER_MOTOR_ROTATE_LEFT_event_handler(uint16_t event_time)
{
    printf("LEFT_event\n");
    if (ProgramTimer_inSec >= event_time + ALTER_MOTOR_ROTATE_LEFT_time)
    {
        //after finishing the event
        ALTERNATING_TIME = ProgramTimer_inSec;//update the event timer
        OFF(LM_PORT, LM_PIN);
        return ALTER_MOTOR_ROTATE_RIGHT_state;
    }
    else
        ON(LM_PORT, LM_PIN); // 20,5,20,5,20,5
    return ALTER_MOTOR_ROTATE_LEFT_state;

}

ALTERNATING_STATEs_e ALTER_FINISH_event_handler(uint16_t event_time){
    printf("FINISH_event\n");
    ALTERNATING_TIME = ProgramTimer_inSec;//update the event timer
    return ALTERNATING_FINISHED_state;
}

//you are not define state sequence 
//to be dynamic software could be edited easily
//we define every event related to its state
// array of struct
ALTERNATING_stateMachine_t ALTERNATING_stateMachine_a[] = {
    /*state1*/ {ALTER_MOTOR_ROTATE_RIGHT_state , ALTER_MOTOR_ROTATE_RIGHT_event , ALTER_MOTOR_ROTATE_RIGHT_event_handler }, // 0 index
    /*state2*/ {ALTER_MOTOR_STOP_state         , ALTER_MOTOR_STOP_event         , ALTER_MOTOR_STOP_event_handler         }, // 1 index
    /*state3*/ {ALTER_MOTOR_ROTATE_LEFT_state  , ALTER_MOTOR_ROTATE_LEFT_event  , ALTER_MOTOR_ROTATE_LEFT_event_handler  }, // 2 index
    /*state4*/ {ALTERNATING_FINISHED_state     , ALTERNATING_FINISHED_event     , ALTER_FINISH_event_handler             }, // 3 index

};
ALTERNATING_stateMachine_t ALTERNATING_stateMachine_SEQUENCE_a[] = {
    /*state1*/ {ALTER_MOTOR_ROTATE_RIGHT_state , ALTER_MOTOR_ROTATE_RIGHT_event , ALTER_MOTOR_ROTATE_RIGHT_event_handler }, // 0 index
    /*state2*/ {ALTER_MOTOR_STOP_state         , ALTER_MOTOR_STOP_event         , ALTER_MOTOR_STOP_event_handler         }, // 1 index
    /*state3*/ {ALTER_MOTOR_ROTATE_LEFT_state  , ALTER_MOTOR_ROTATE_LEFT_event  , ALTER_MOTOR_ROTATE_LEFT_event_handler  }, // 2 index
    /*state4*/ {ALTER_MOTOR_STOP_state         , ALTER_MOTOR_STOP_event         , ALTER_MOTOR_STOP_event_handler         }, // 3 index
    /*state5*/ {ALTERNATING_FINISHED_state     , ALTERNATING_FINISHED_event     , ALTER_FINISH_event_handler             }, // 4 index

};
//

// start state
ALTERNATING_STATEs_e ALTER_nextState = ALTER_MOTOR_ROTATE_RIGHT_state;
void INIT_ALTERNATING(void)
{
    printf("AlternInit\n");
    OFF(RM_PORT, RM_PIN);
    OFF(LM_PORT, LM_PIN);
    ALTERNATING_TIME = ProgramTimer_inSec;
}

void ALTERNATING_RUN(void)//AS A SUB_STATEMACHINE
{
  
    if (ALTER_nextState < ALTERNATING_FINISHED_state ){
        // call related function to next state ,ask it for new transition
        // excecute next event when returned state changed
        ALTER_nextState = (*ALTERNATING_stateMachine_a[ALTER_nextState].ALTER_handler)(ALTERNATING_TIME);
    }
}

void MAINPROGRAM_RUN(void){

    ALTERNATING_RUN();
}
int main(int argc, char const *argv[])
{
    INIT_ALTERNATING();

    while (1)
    {
        printf("\nHI:%d\n", ProgramTimer_inSec);
        ProgramTimer_inSec++;
        delay(500);
        ALTERNATING_RUN();
        if(ProgramTimer_inSec > 50)break;
    }
    return 0;
}
