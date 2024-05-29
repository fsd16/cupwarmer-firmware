/* 
 * File:   fault.h
 * Author: Finn
 *
 * Created on 29 May 2024, 12:35 AM
 */

#ifndef FAULT_H
#define	FAULT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "mcc_generated_files/timer/delay.h"
#include "mcc_generated_files/system/pins.h"
    
void faultHandler(void);

#ifdef	__cplusplus
}
#endif

#endif	/* FAULT_H */

