#include "rios.h"

#define MAX_TASKS 5
#define TIMER_DIV 64
#define TIMER_FREQ_FP fixedpt_fromint(F_CPU)/TIMER_DIV
#define TIMER_TOP_FP fixedpt_fromint(0x1457)

uint8_t definedTasksNum;

//task* tasks = NULL;
task tasks[MAX_TASKS];

uint8_t TickFlag = 0;

void TickISR(void) {
   if (!TickFlag) {
      TickFlag = 1;
   }
   // iF TickFlag is still true then the timer ticked before the task was complete
   return;
}

void RIOS_Initialize()
{
    TCA0_OverflowCallbackRegister(&TickISR);

//    tasks = (task*)malloc(MAX_TASKS * sizeof(task));
//    if (tasks == NULL) {
//        // Handle allocation failure
//        while (1);
//    }
   
}

task* RIOS_DefineTask(bool enabled, int32_t freq, RIOS_func func)
{
    if (definedTasksNum < MAX_TASKS) {
        fixedpt period_fp = TIMER_FREQ_FP / freq;
        tasks[definedTasksNum] = (task){
            .enabled = enabled,
            .period_fp = period_fp,
            .elapsedTime_fp = period_fp,
            .TickFct = func,
        };
        return &tasks[definedTasksNum++];
    } else {
        faultHandler();
        
    }
    return NULL;
    
}

void RIOS_Run(void)
{
    TCA0_Start();
    while(1)
    {
        uint8_t i = 0;
        // Heart of the scheduler code
        for (i=0; i < definedTasksNum; ++i) {
            if (!tasks[i].enabled) {
                // Skip task
                continue;
            }
            
            if (tasks[i].elapsedTime_fp >= tasks[i].period_fp) { // Ready
                tasks[i].TickFct(); //execute task tick
                tasks[i].elapsedTime_fp = 0;
            }
            tasks[i].elapsedTime_fp += TIMER_TOP_FP;
        }
        TickFlag = 0;
        while (!TickFlag) {
           DELAY_microseconds(1);
        }        
    }
}