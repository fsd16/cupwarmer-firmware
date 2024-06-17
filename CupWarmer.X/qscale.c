
#include "qscale.h"

// Function to add two Q32 values
q32_t q32_add(q32_t a, q32_t b) {
    return a + b;
}

// Function to subtract two Q32 values
q32_t q32_sub(q32_t a, q32_t b) {
    return a - b;
}

// Function to multiply two Q32 values
q32_t q32_mul(q32_t a, q32_t b) {
    // Perform the multiplication and then right shift by 32 to scale back
    return (q32_t)(((int64_t)a * b) >> 32);
}

// Function to divide two Q32 values
q32_t q32_div(q32_t a, q32_t b) {
    // Perform the division by first left shifting by 32 to retain precision
    return (q32_t)(((int64_t)a << 32) / b);
}