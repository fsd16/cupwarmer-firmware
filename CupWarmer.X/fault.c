#include "fault.h"

void faultHandler(void)
{   
    // Toggle PA1 output
    while(1) {
        IO_PA1_SetHigh();
        DELAY_milliseconds(500);
        IO_PA1_SetLow();
        DELAY_milliseconds(500);
        IO_PA1_SetHigh();
        DELAY_milliseconds(500);
        IO_PA1_SetLow();
        DELAY_milliseconds(1000);
    }
}