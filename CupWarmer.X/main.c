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
#include "mcc_generated_files/adc/adc0.h"
#include "mcc_generated_files/dac/dac0.h"
#include "rios.h"

/*
    Main application
*/

#define DAC_VREF 4.34
#define ADC_VREF 4.34
#define DAC_RESOLUTION 10
#define ADC_RESOLUTION 10
#define VoltageToDAC(voltage) (voltage * ((1 << DAC_RESOLUTION) - 1)/DAC_VREF)
#define ADCToVoltage(adc_value) (adc_value * ADC_VREF / ((1 << ADC_RESOLUTION) - 1))

#define STATIC_POINT_SCALE 22
#define StaticPointScaleUp(num) (num*(1U << STATIC_POINT_SCALE))
#define StaticPointScaleDown(num) (num/(1U << STATIC_POINT_SCALE))

// Constants for PI controller
#define KP 20   // Proportional gain
#define KI 1   // Integral gain
#define MAX_INTEGRAL 100  // Maximum intergral term
#define MIN_INTEGRAL -MAX_INTEGRAL   // Minimum intergral term
#define MAX_OUTPUT 1023
#define MIN_OUTPUT 0

// Task rates (Hz))
#define LED_TOGGLE_FREQ 10
#define PI_UPDATE_FREQ 10

void Tick_LEDToggle(void);;
void Tick_ControllerUpdate(void);

volatile bool pb_toggle_state = false;
volatile bool pb_pressed_state = false;
volatile int16_t feedback_m = 10;
volatile int16_t error_m = 10;
volatile int16_t setpoint_m = 10;

void PB_State(void)
{
    cpu_irq_disable();
    pb_pressed_state = IO_PA7_GetValue();
    if (!IO_PA7_GetValue()){
        // Button is released edge
        pb_toggle_state = !pb_toggle_state ;
        IO_PA1_Toggle();
    }
    cpu_irq_enable();
}

int main(void)
{
    SYSTEM_Initialize();
    RIOS_Initialize();
    
    IO_PA7_SetInterruptHandler(&PB_State);
    
    //Create tasks in order of priority
    task* task_ledToggle = RIOS_DefineTask(true, LED_TOGGLE_FREQ, &Tick_LEDToggle);
    task* controllerToggle = RIOS_DefineTask(true, PI_UPDATE_FREQ, &Tick_ControllerUpdate);
    
    RIOS_Start();
            
    while(1)
    {
//        RIOS_Run(); // Will block
        Tick_ControllerUpdate();
//        monitor = ADC0_GetConversion(ADC0_IO_PA3);
        DELAY_milliseconds(100);
        Tick_LEDToggle();
    }    
}

void Tick_LEDToggle(void)
{   
    // Toggle PA1 output
    IO_PA1_Toggle();
    
}

#define ALPHA 5
#define GAIN 1

void Tick_ControllerUpdate(void) {
    
    // Static variable for the integral of the error (initialized once)
    static int32_t error_sum = 0;
    
    // Variables
    int16_t error;
    int16_t setpoint;
    static int16_t feedback;
    int16_t proportional;
    int16_t integral;
    int16_t output;

    // Set the desired setpoint and feedback values (example)
    feedback_m = feedback;
    setpoint = ADC0_GetConversion(ADC0_IO_PA3); // Would be good to filter these
    setpoint_m = setpoint;
//    feedback = ADC0_GetConversion(ADC0_IO_PA2);

    // Calculate error
    error = setpoint - feedback;
    error_m = error;
    // Proportional term
    proportional = KP * error;

    // Integral term with anti-windup
    // TODO: look into trap rule as well as + 1/2 for trunc rounding
    error_sum += StaticPointScaleDown(error * (StaticPointScaleUp(1)/ PI_UPDATE_FREQ)); //Max division error is 1
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
//    DAC0_SetOutput(output);
    feedback = (ALPHA*feedback + (10-ALPHA)*GAIN*output)/10;
}