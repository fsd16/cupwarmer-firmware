#include "rios.h"

#define MAX_TASKS 5
#define TIMER_DIV 64
#define TIMER_FREQ F_CPU/TIMER_DIV
#define TIMER_TOP 0x1457

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

task* RIOS_DefineTask(bool enabled, uint32_t freq, RIOS_func func)
{
    if (definedTasksNum < MAX_TASKS) {
        uint32_t period = (TIMER_FREQ + freq/2) / freq;
        tasks[definedTasksNum] = (task){
            .enabled = enabled,
            .period = period,
            .elapsedTime = period,
            .TickFct = func,
        };
        return &tasks[definedTasksNum++];
    } else {
        faultHandler();
        
    }
    return NULL;
    
}

void RIOS_Start(void)
{
    TCA0_Start();
}

void RIOS_Run(void)
{
    while(1)
    {
        uint8_t i = 0;
        // Heart of the scheduler code
        for (i=0; i < definedTasksNum; ++i) {
            if (tasks[i].enabled) {
                if (tasks[i].elapsedTime >= tasks[i].period) { // Ready
                    tasks[i].TickFct(); //execute task tick
                    tasks[i].elapsedTime = 0;
                }
                tasks[i].elapsedTime += TIMER_TOP;
            }
        }
        TickFlag = 0;
        while (!TickFlag) {
           DELAY_milliseconds(1);
        }        
    }
}