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
#include "fixedptc.h"

/*
    Main application
*/

// Task rates (Hz))
#define LED_TOGGLE_FREQ 1
#define PI_UPDATE_FREQ 10
#define ENABLE_OUTPUT_FREQ 1

// Constants and macros for DAC
#define DAC_VREF_FP fixedpt_fromint(4.34)
#define DAC_RESOLUTION 8
#define DAC_MAX_BIN_FP fixedpt_fromint((1 << DAC_RESOLUTION) - 1)
#define DAC_VREF_RATIO_FP fixedpt_div(DAC_MAX_BIN_FP, DAC_VREF_FP)

// Constants and macros for ADC
#define ADC_VREF_FP fixedpt_fromint(4.34)
#define ADC_RESOLUTION 10
#define ADC_MAX_BIN_FP fixedpt_fromint((1 << ADC_RESOLUTION) - 1)
#define VREF_ADC_RATIO_FP fixedpt_div(ADC_VREF_FP, ADC_MAX_BIN_FP)

// Constants for feedback calcs
#define FB_VREF_FP fixedpt_fromfloat(0.8)

#define R1_FP fixedpt_fromint(24)
#define R2_FP fixedpt_fromfloat(1.02258001)
#define R3_FP fixedpt_fromfloat(4.34)

#define R1_div_R2 fixedpt_div(R1_FP, R2_FP)
#define R1_div_R3 fixedpt_div(R1_FP, R3_FP)

#define VOUT_OFFSET_FP fixedpt_mul((1+R1_div_R2+R1_div_R3), FB_VREF_FP)

// Constants for PI controller
#define PI_UPDATE_PERIOD_FP fixedpt_fromint(1)/PI_UPDATE_FREQ
#define KP 20   // Proportional gain
#define KI 1   // Integral gain
#define MAX_INTEGRAL_10B_FP fixedpt_fromint(100)  // Maximum intergral term
#define MIN_INTEGRAL_10B_FP -MAX_INTEGRAL_10B_FP   // Minimum intergral term
#define MAX_OUTPUT 1023
#define MIN_OUTPUT 0



void Tick_LEDToggle(void);;
void Tick_ControllerUpdateOpen(void);
void Tick_ControllerUpdate(void);
void Tick_EnableOutput(void);

volatile bool pb_toggle_state = false;
volatile bool pb_pressed_state = false;
volatile fixedpt result_fp_m;

fixedpt VoltageToDAC_FP(fixedpt voltage) {
    return fixedpt_mul(voltage, DAC_VREF_RATIO_FP);
}

fixedpt ADCToVoltage_FP(fixedpt bin) {
    return fixedpt_mul(bin, VREF_ADC_RATIO_FP);
}


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
    
//    IO_PA7_SetInterruptHandler(&PB_State);
    
    //Create tasks in order of priority
    RIOS_DefineTask(true, PI_UPDATE_FREQ, &Tick_ControllerUpdateOpen);
    RIOS_DefineTask(true, LED_TOGGLE_FREQ, &Tick_LEDToggle);
    RIOS_DefineTask(true, ENABLE_OUTPUT_FREQ, &Tick_EnableOutput);
            
    while(1)
    {
        Tick_ControllerUpdateOpen();
        RIOS_Run();
    }    
}

void Tick_LEDToggle(void)
{   
    // Toggle PA1 output
    IO_PA1_Toggle();
    
}

void Tick_ControllerUpdateOpen(void) {
    int16_t setpoint_10b;
    
    setpoint_10b = ADC0_GetConversion(ADC0_IO_PA3);
    DAC0_SetOutput(setpoint_10b >> 2); // Convert to 8 bit before setting output
    
//    result_fp_m = VOUT_OFFSET_FP - ADCToVoltage_FP(R1_div_R3*setpoint_10b); //Can do this as setpoint is an int

}

void Tick_ControllerUpdate(void) {
    
    // Static variable for the integral of the error (initialized once)
    static int32_t error_sum_10b_fb = 0;
    
    // Variables
    int16_t error_10b;
    int16_t setpoint_10b;
    static int16_t feedback_10b;
    int16_t proportional_10b_fp;
    int16_t integral_10b_fp;
    int16_t output_10b;

    // Set the desired setpoint and feedback values (example)
    setpoint_10b = ADC0_GetConversion(ADC0_IO_PA3); // Would be good to filter these
    feedback_10b = ADC0_GetConversion(ADC0_IO_PA2);

    // Calculate error
    error_10b = setpoint_10b - feedback_10b;
    // Proportional term
    proportional_10b_fp = fixedpt_fromint(KP * error_10b);

    // Integral term with anti-windup
    // TODO: look into trap rule as well as + 1/2 for trunc rounding
    error_sum_10b_fb += error_10b * PI_UPDATE_PERIOD_FP; //Max division error is 1
    if (error_sum_10b_fb > MAX_INTEGRAL_10B_FP) {
        error_sum_10b_fb = MAX_INTEGRAL_10B_FP;
    } else if (error_sum_10b_fb < MIN_INTEGRAL_10B_FP) {
        error_sum_10b_fb = MIN_INTEGRAL_10B_FP;
    }
    integral_10b_fp = KI * error_sum_10b_fb;

    // Calculate controller output
    output_10b = fixedpt_toint(proportional_10b_fp + integral_10b_fp);

    // Limit output to maximum and minimum values
    if (output_10b > MAX_OUTPUT) {
        output_10b = MAX_OUTPUT;
    } else if (output_10b < MIN_OUTPUT) {
        output_10b = MIN_OUTPUT;
    }

    // Set the output value
//    DAC0_SetOutput(output_10b);
}

void Tick_EnableOutput(void) {
    static int16_t setpoint_10b;
    
    setpoint_10b = ADC0_GetConversionResult();
    
    if (setpoint_10b < 188) {
        IO_PA7_SetLow();
    } else {
        IO_PA7_SetHigh();
    }

}