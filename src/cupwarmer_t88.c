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
#include "SoftwareSerial.h"

// Boolean values
#define true 1
#define false 0

// Constants for PI controller
#define KP 20   // Proportional gain
#define KI 0.1   // Integral gain

// Constants for intergral
#define MIN_INT -100.0    // Minimum intergral term
#define MAX_INT 100.0  // Maximum intergral term

// Constants for temperature sensor conversion
#define SENSOR_MIN_VOLTAGE 0.14   // Minimum voltage from temperature sensor
#define SENSOR_MAX_VOLTAGE 1.71   // Maximum voltage from temperature sensor
#define TEMP_MIN -40.0             // Minimum temperature range
#define TEMP_MAX 125.0           // Maximum temperature range

// Constants for PWM duty cycle
#define PWM_PERIOD 1000.0   // PWM period in microseconds
#define PWM_MIN_DUTY 0.0    // Minimum duty cycle
#define PWM_MAX_DUTY 100.0  // Maximum duty cycle

char* intToStr(int number, char* buffer) {

    if (buffer == NULL) {
        // Handle invalid buffer
        return;
    }

    // Check if the number is negative
    int isNegative = 0;
    if (number < 0) {
        isNegative = 1;
        number = -number;
    }

    // Convert each digit to a character and store in the buffer
    int index = 0;
    do {
        buffer[index++] = '0' + (number % 10);
        number /= 10;
    } while (number > 0);

    // Add negative sign if necessary
    if (isNegative) {
        buffer[index++] = '-';
    }

    // Reverse the string in-place
    int i = 0;
    int j = index - 1;
    while (i < j) {
        char temp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = temp;
        i++;
        j--;
    }

    // Null-terminate the string
    buffer[index] = '\0';
}

void floatToStr(float number, char* buffer, int precision)
{
    if (buffer == NULL) {
        // Handle invalid buffer
        return;
    }

    // Check if the number is negative
    int isNegative = 0;
    if (number < 0) {
        isNegative = 1;
        number = -number;
    }

    // Extract the integer and fractional parts of the number
    int integerPart = (int)number;
    float fractionalPart = number - integerPart;

    // Convert the integer part to a string
    intToStr(integerPart, buffer);

    // Find the length of the integer part
    int len = 0;
    while (buffer[len] != '\0') {
        len++;
    }

    // Add the decimal point to the string
    buffer[len] = '.';
    len++;

    // Convert the fractional part to a string with the specified precision
    for (int i = 0; i < precision; i++) {
        fractionalPart *= 10;
        int digit = (int)fractionalPart;
        buffer[len] = '0' + digit;
        len++;
        fractionalPart -= digit;
    }

    // Add negative sign if necessary
    if (isNegative) {
        buffer[len] = '-';
        len++;
    }

    // Null-terminate the string
    buffer[len] = '\0';
}

void sendStr(const char* str, bool CRLF)
{
    if (str == NULL) {
        // Handle invalid input
        return;
    }

    int i = 0;
    while (str[i] != '\0') {
        softSerialWrite(str[i]);
        i++;
    }
    
    if (CRLF) {
        softSerialWrite(0x0D);
        softSerialWrite(0x0A);
    }
}

void sendFloat(float value, int precision, bool CRLF) {
    char str_buffer[20];
    
    floatToStr(value, str_buffer, precision);
	sendStr(str_buffer, CRLF);
}

// Model for resistive element
//-----------------------------------------------------------------------------------

#include <math.h>

#define THERMAL_RESISTANCE 0.2     // Thermal resistance of the element in degC/W
#define HEAT_CAPACITY 20*1.3       // Heat capacity of the element in J/degC
#define AMBIENT_TEMPERATURE 15.0   // Ambient temperature in degC
#define FUDGE_FACTOR 3.8
#define TIME_CONSTANT 104.0          // Time constant in seconds
#define DELAY_MILLISECONDS 10      // Delay between temperature updates in milliseconds

void model(float duty_cycle, float* temperature, float* time)
{
    float V, t, R, m, C, h, A;
    float delta_T_sat;
    float delta_T_gain;
    float delta_T_loss;
    float delta_T;
    // static float temperature = AMBIENT_TEMPERATURE;
    // float time = 0.0;

    V = 12.0;
    t = 1.0;
    R = 1.25*0.017; // Fudge to increase temperature gain to better overcome loss to env....
    m = 0.020;
    C = 1300.0;
    h = 10.0*0.25; // Fudge to increase time consant to match reality more closely
    A = 0.050;

    delta_T_sat = 1.0/10000.0 * (duty_cycle * duty_cycle) * (V * V * t) / (R * m * C);
    delta_T_gain = delta_T_sat * (1 - exp(-t * h * A / (m * C)));

    delta_T_loss = -(*temperature - AMBIENT_TEMPERATURE) * (1 - exp(-t * h * 0.5 * A / (m * C)));

    delta_T = delta_T_gain + delta_T_loss;

    *temperature += delta_T;
    *time += t;
    // sendStr("deltas: ", false);
    // sendFloat(delta_T_gain, 3, false);
    // sendStr(", ", false);
    // sendFloat(delta_T_loss, 3, true);
}

//-----------------------------------------------------------------------------------

// Function to convert temperature sensor voltage to temperature
float convertVoltageToTemperature(float voltage) {
    float temperature = ((voltage - SENSOR_MIN_VOLTAGE) * 1 / (SENSOR_MAX_VOLTAGE - SENSOR_MIN_VOLTAGE)) * (TEMP_MAX - TEMP_MIN) + TEMP_MIN;
    return temperature;
}

// Function to calculate the PI control output
float calculateDutyCycle(float setpoint, float temperature, float* integral_term) {
    float error = setpoint - temperature;
    float proportional_term = KP * error;
    *integral_term += KI * error;

    
    // Limit integral term to prevent windup
    if (*integral_term > MAX_INT) {
        *integral_term = MAX_INT;
    } else if (*integral_term < MIN_INT) {
        *integral_term = MIN_INT;
    }

    sendFloat(*integral_term, 1, false);
    sendStr(", ", false);
    sendFloat(proportional_term, 1, true);

    // Calculate duty cycle
    float duty_cycle = proportional_term + *integral_term;

    // Limit duty cycle within PWM range
    if (duty_cycle > PWM_MAX_DUTY) {
        duty_cycle = PWM_MAX_DUTY;
    } else if (duty_cycle < PWM_MIN_DUTY) {
        duty_cycle = PWM_MIN_DUTY;
    }

    return duty_cycle;
}

bool stableCheck(float value, int stable_count, int precision) {
    static int prev_value = 0;
    static int count = 0;
    bool stable = false;

    float scale = pow(10, precision);
    // float scale = 1.0;

    int scaled_value = (int) (value*scale);

    if (scaled_value==prev_value) {
        count++;
    } else {
        count = 0;
    }

    prev_value = scaled_value;
    
    if (count >= stable_count) {
        stable = true;
    }

    return stable;
}

int main() {

    // Simulated inputs for demonstration
    float setpoint_temperature = 25.0;  // Setpoint temperature in degC
    // float sensor_voltage = 0.66;    // Voltage from temperature sensor

    // Convert sensor voltage to temperature
    // float temperature = convertVoltageToTemperature(sensor_voltage);
    float temperature = AMBIENT_TEMPERATURE;
    float time = 0.0;

    // Variables for PI control
    float integral_term = 0.0;
    float duty_cycle = 0.0;

    // Set built-in LED pin as output
    pin_config(LED, OUTPUT);
    
    softSerialBegin(9600);
    while (!softSerialAvailable()) {
        _delay_ms(10);
    }

    softSerialRead();

    while (1) {
        // Measure the current temperature
        model(duty_cycle, &temperature, &time);
        // Calculate the new duty cycle based on the current temperature.
        duty_cycle = calculateDutyCycle(setpoint_temperature, temperature, &integral_term);

        // sendFloat(time, 1, false);
        // sendStr(", ", false);
        // sendFloat(duty_cycle, 1, false);
        // sendStr(", ", false);
        // sendFloat(temperature, 1, true);
        
        
        // if ((int)duty_cycle != 100 && (int)duty_cycle != 0 && stableCheck(duty_cycle, 10, 1)) {
        if ((int)time==160){
            break;
        }
    }
    return 0;
}