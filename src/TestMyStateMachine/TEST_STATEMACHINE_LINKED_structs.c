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


typedef enum {
    state1,
    state2,
}SUB_STRUCT1_STATES_e;

//SMALLEST SUBPROGRAM
typedef struct 
{   

    
}SUB_STRUCT1_t;


//THEN LARGEST ONE AND LINK IT WITH PREVIOUS ONE
typedef struct 
{
    SUB_STRUCT1_t *PREVSTRUCT;//LINK PREVIOUS STRUCT
}SUB_STRUCT2_t;


//THEN LARGEST ONE AND LINK IT WITH PREVIOUS ONE
typedef struct 
{
    SUB_STRUCT2_t *PREVSTRUCT;//LINK PREVIOUS STRUCT

}MAIN_PROGRAM_STRUCT_t;


void MAIN_PROGRAM_STRUCT_RUN(void){


}