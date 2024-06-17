/* 
 * File:   qscale.h
 * Author: Finn
 *
 * Created on 16 June 2024, 8:08 PM
 */

#ifndef QSCALE_H
#define	QSCALE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

// Define the Q32 fixed-point type
typedef int64_t q32_t;

#define Q32_ONE  (1LL << 32)
#define Q32_HALF (1LL << 31)

// Macro to convert an integer to Q32 format
#define TO_Q32(n) ((q32_t)((n) * (1LL << 32)))

// Macro to convert a Q32 value to an integer (truncating the fractional part)
#define TO_INT(q) ((int)((q) >> 32))

// Function to add two Q32 values
q32_t q32_add(q32_t a, q32_t b);

// Function to subtract two Q32 values
q32_t q32_sub(q32_t a, q32_t b);

// Function to multiply two Q32 values
q32_t q32_mul(q32_t a, q32_t b);

// Function to divide two Q32 values
q32_t q32_div(q32_t a, q32_t b);

#ifdef	__cplusplus
}
#endif

#endif	/* QSCALE_H */

