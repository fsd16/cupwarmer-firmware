#ifndef __AVR_ATtiny88__
#define __AVR_ATtiny88__
#endif

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
// #include "printf.h"
#include "pins.h"
#include "attiny88.h"

int main()
{
    // Set built-in LED pin as output
    pin_config(LED, OUTPUT);

    while (1) {
        pin_set(LED, HIGH);
        // printf_("High");
        _delay_ms(100);
        pin_set(LED, LOW);
        // printf_("Low");
        _delay_ms(100);
    }
    return 0;
}
