 /*
 * MAIN Generated Driver File
 * 
 * @file main.c
 * 
 * @defgroup main MAIN
 * 
 * @brief This is the generated driver implementation file for the MAIN driver.
 *
 * @version MAIN Driver Version 1.0.0
*/

/*
© [2024] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/
#include "mcc_generated_files/system/system.h"
#include "mcc_generated_files/timer/delay.h"
#include "mcc_generated_files/system/pins.h"
#include "mcc_generated_files/dac/dac0.h"
#include "rios.h"
/*
    Main application
*/

// Constants for PI controller
#define KP 20   // Proportional gain
#define KI 1   // Integral gain
#define MAX_INTEGRAL 100  // Maximum intergral term
#define MIN_INTEGRAL -MAX_INTEGRAL   // Minimum intergral term
#define MAX_OUTPUT 1023
#define MIN_OUTPUT 0

// Task rates (Hz))
#define LED_TOGGLE_FREQ 10
#define PI_UPDATE_FREQ 100

void Tick_LEDToggle(void);;
void Tick_ControllerUpdate(void);

int main(void)
{
    SYSTEM_Initialize();
    RIOS_Initialize();
    
    //Create tasks in order of priority
    RIOS_DefineTask(LED_TOGGLE_FREQ, &Tick_LEDToggle);
    RIOS_DefineTask(PI_UPDATE_FREQ, &Tick_ControllerUpdate);
    
    RIOS_Start();
            
    while(1)
    {
        RIOS_Run(); // Will block
    }    
}

void Tick_LEDToggle(void)
{   
    // Toggle PA1 output
    IO_PA1_Toggle();
}

void Tick_ControllerUpdate(void) {
    
    // Static variable for the integral of the error (initialized once)
    static int16_t error_sum = 0;
    
    // Variables
    int16_t error;
    int16_t setpoint;
    int16_t feedback;
    int16_t proportional;
    int16_t integral;
    int16_t output;

    // Set the desired setpoint and feedback values (example)
    setpoint = ADC0_GetConversion(ADC0_IO_PA3);
    feedback = ADC0_GetConversion(ADC0_IO_PA2);

    // Calculate error
    error = setpoint - feedback;

    // Proportional term
    proportional = KP * error;

    // Integral term with anti-windup
    // TODO: look into trap rule as well as + 1/2 for trunc rounding
    error_sum += error * (1.0 / PI_UPDATE_FREQ); // Ensure floating-point division
    if (error_sum > MAX_INTEGRAL) {
        error_sum = MAX_INTEGRAL;
    } else if (error_sum < MIN_INTEGRAL) {
        error_sum = MIN_INTEGRAL;
    }
    integral = KI * error_sum;

    // Calculate controller output
    output = proportional + integral;

    // Limit output to maximum and minimum values
    if (output > MAX_OUTPUT) {
        output = MAX_OUTPUT;
    } else if (output < MIN_OUTPUT) {
        output = MIN_OUTPUT;
    }

    // Set the output value
    DAC0_SetOutput(output);
}