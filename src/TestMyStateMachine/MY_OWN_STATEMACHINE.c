// Try to apply state machine concept
// Washing machine have diffrent program every program have some of main subprograms(wash,rinse,prewash)
// every subprogram has parameters dependent on every program (as:Time,loopTimes,sequence,)
// you have user input choose buttons should be carious about it all time

// Try in this version to be dynamic in your code
// If any edits needed just edit in main structure so put it in your mind, mohamed

#include "ERRORS.h"
#include "API.h"
#include "alternatingTask.h"
#include "waterLevel_FILLTask.h"
#include "USER_BUTTONS.h"

volatile float ProgramTimer_inSec = 0; // 65,535//max here about 18 hour //max needed 60*60*24=86400// main timer in seconds ,starts when user press start program after choosing it used for alternating statemachine and other
volatile uint8_t ALTERNATING_START_POINT = 1;
volatile uint8_t WATERLEVELFILLFULL_START_POINT = 0;

void MAINPROGRAM_RUN(void)
{
    ALTERNATING_RUN();
}
int main(int argc, char const *argv[])
{
    
    INIT_ALTERNATING();
    INIT_WATERLEVELFILLFULL();
    int x=0;
    printf("%c\n__",getchar());
    
    while (1){
    
        PRINTDEBUG_Vf("\nTime:",ProgramTimer_inSec)
        ProgramTimer_inSec+=1;
        delay(100);
        ALTERNATING_RUN();
        WATERLEVELFILLFULL_FILLING_RUN();

        /*
        if (ProgramTimer_inSec > 50)//test end
            ALTERNATING_END();
            */
        if (ProgramTimer_inSec > 4*60)
            break;
    }
    return 0;
}
