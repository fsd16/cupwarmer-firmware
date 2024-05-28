/* 
 * File:   rios.h
 * Author: Finn
 *
 * Created on May 25, 2024, 8:14 PM
 */

#ifndef RIOS_H
#define	RIOS_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "mcc_generated_files/system/utils/compiler.h"
#include "mcc_generated_files/timer/delay.h"
#include "mcc_generated_files/timer/tca0.h"
#include "mcc_generated_files/system/clock.h"
    
typedef void (*RIOS_func)(void);

typedef struct task {
   uint32_t period;      // Rate at which the task should tick
   uint32_t elapsedTime; // Time since task's last tick
   void (*TickFct)(void);     // Function to call for task's tick
} task;

void RIOS_Initialize();

void RIOS_DefineTask(uint32_t freq, RIOS_func func);

void RIOS_Start(void);

void RIOS_Run(void);


#ifdef	__cplusplus
}
#endif

#endif	/* RIOS_H */

