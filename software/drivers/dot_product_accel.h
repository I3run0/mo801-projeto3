#ifndef __DOT_PRODUCT_ACCEL_H
#define __DOT_PRODUCT_ACCEL_H

#include <stdint.h>
#include <generated/csr.h>
#include <stddef.h>

// Hardware accelerator configuration
#define DOT_PRODUCT_ACCEL_INPUT_SIZE 4

// Check if accelerator is available
#ifdef CSR_DOT_PRODUCT_ACCEL_BASE
#define DOT_PRODUCT_ACCEL_AVAILABLE 1
#else
#define DOT_PRODUCT_ACCEL_AVAILABLE 0
#endif

// Function declarations
double logistic_accel_dot_product(size_t size, double *inputs, double *weights);

// Utility functions for fixed-point conversion
int32_t double_to_fixed(double value, int fractional_bits);

double fixed_to_double(int32_t value, int fractional_bits);

#endif /* __DOT_PRODUCT_ACCEL_H */