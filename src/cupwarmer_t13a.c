// TODO: Heavy optimisation
// Note: Currently using -Ofast optimisation. Results in the best size reduction, but is risky.

#ifndef __AVR_ATtiny88__
#define __AVR_ATtiny88__
#endif

#include <stdio.h>
// #include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include "pins.h"
#include "attiny88.h"

// Constants for PI controller
#define KP 20   // Proportional gain
#define KI 1   // Integral gain

// Constants for intergral
#define MIN_INT -100   // Minimum intergral term
#define MAX_INT 100  // Maximum intergral term

// Constants for temperature sensor conversion
#define SENSOR_MIN_VOLTAGE 1   // Minimum voltage from temperature sensor
#define SENSOR_MAX_VOLTAGE 2   // Maximum voltage from temperature sensor
#define TEMP_MIN -40             // Minimum temperature range
#define TEMP_MAX 125           // Maximum temperature range

// Constants for PWM duty cycle
#define PWM_PERIOD 1000   // PWM period in microseconds
#define PWM_MIN_DUTY 0    // Minimum duty cycle
#define PWM_MAX_DUTY 100  // Maximum duty cycle

// Function to convert temperature sensor voltage to temperature
int convertVoltageToTemperature(int voltage) {
    int temperature = ((voltage - SENSOR_MIN_VOLTAGE) * 1 / (SENSOR_MAX_VOLTAGE - SENSOR_MIN_VOLTAGE)) * (TEMP_MAX - TEMP_MIN) + TEMP_MIN;
    return temperature;
}

// Function to calculate the PI control output
int calculateDutyCycle(int setpoint, int temperature, int* integral_term) {
    int error = setpoint - temperature;
    int proportional_term = KP * error;
    *integral_term += KI * error * 1/10;

    
    // Limit integral term to prevent windup
    if (*integral_term > MAX_INT) {
        *integral_term = MAX_INT;
    } else if (*integral_term < MIN_INT) {
        *integral_term = MIN_INT;
    }

    // Calculate duty cycle
    int duty_cycle = proportional_term + *integral_term;

    // Limit duty cycle within PWM range
    if (duty_cycle > PWM_MAX_DUTY) {
        duty_cycle = PWM_MAX_DUTY;
    } else if (duty_cycle < PWM_MIN_DUTY) {
        duty_cycle = PWM_MIN_DUTY;
    }

    return duty_cycle;
}

int main() {

    // Simulated inputs for demonstration
    int setpoint_temperature = 25;  // Setpoint temperature in degC
    int sensor_voltage = 0;    // Voltage from temperature sensor

    // Convert sensor voltage to temperature
    int temperature = 0;

    // Variables for PI control
    int integral_term = 0;
    int duty_cycle = 0;

    // Set built-in LED pin as output
    pin_config(LED, OUTPUT);

    while (1) {
        // Measure the current temperature
        temperature = convertVoltageToTemperature(sensor_voltage);
        // Calculate the new duty cycle based on the current temperature.
        duty_cycle = calculateDutyCycle(setpoint_temperature, temperature, &integral_term);
        
    }
    return 0;
}