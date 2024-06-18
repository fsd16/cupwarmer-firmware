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
    
#include <stdio.h>
#include "mcc_generated_files/system/utils/compiler.h"
#include "mcc_generated_files/timer/delay.h"
#include "mcc_generated_files/timer/tca0.h"
#include "mcc_generated_files/system/clock.h"
#include "fault.h"
#include "fixedptc.h"
    
typedef void (*RIOS_func)(void);

typedef struct task {
    bool enabled;
    fixedpt period_fp;      // Rate at which the task should tick
    fixedpt elapsedTime_fp; // Time since task's last tick
    void (*TickFct)(void);     // Function to call for task's tick
} task;

void RIOS_Initialize();

task* RIOS_DefineTask(bool enabled, int32_t freq, RIOS_func func);

void RIOS_Run(void);


#ifdef	__cplusplus
}
#endif

#endif	/* RIOS_H */

