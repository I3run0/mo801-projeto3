#include "dot_product_accel_driver.h"
#include <stddef.h>
#include <generated/csr.h>

// Fixed-point arithmetic configuration (Q16.16 format)
#define FIXED_POINT_FRACTIONAL_BITS 16
#define FIXED_POINT_SCALE (1 << FIXED_POINT_FRACTIONAL_BITS)

int32_t double_to_fixed(double value, int fractional_bits);
double fixed_to_double(int32_t value, int fractional_bits);
void logistic_accel_init(void);
void logistic_accel_set_inputs(double *inputs, int count);
void logistic_accel_set_weights(double *weights, int count);
int32_t logistic_accel_get_result(void);
int32_t logistic_accel_compute_chunk(double *inputs, double *weights, int chunk_size);
double logistic_accel_dot_product(size_t size, double *inputs, double *weights);

// CSR access functions for logistic accelerator inputs and weights
#if LOGISTIC_ACCEL_AVAILABLE

// Input CSR write functions
static inline void logistic_input_write(uint32_t *values) {
    for (int i = 0; i < LOGISTIC_ACCEL_INPUT_SIZE; i++) {
        csr_write_simple(values[i], CSR_LOGISTIC_INPUT_ADDR + (i * 4));
    }
}

// Weight CSR write functions  
static inline void logistic_weight_write(uint32_t *values) {
    for (int i = 0; i < LOGISTIC_ACCEL_INPUT_SIZE; i++) {
        csr_write_simple(values[i], CSR_LOGISTIC_WEIGHT_ADDR + (i * 4));
    }
}

#endif

/**
 * Initialize the logistic regression accelerator
 */
void logistic_accel_init(void) {
#if LOGISTIC_ACCEL_AVAILABLE
    // Clear all input and weight registers
    uint32_t zeros[LOGISTIC_ACCEL_INPUT_SIZE] = {0};
    logistic_input_write(zeros);
    logistic_weight_write(zeros);
#endif
}

/**
 * Convert double to fixed-point representation
 */
int32_t double_to_fixed(double value, int fractional_bits) {
    return (int32_t)(value * (1 << fractional_bits));
}

/**
 * Convert fixed-point to double representation
 */
double fixed_to_double(int32_t value, int fractional_bits) {
    return (double)value / (1 << fractional_bits);
}

/**
 * Set input values in the accelerator
 */
void logistic_accel_set_inputs(double *inputs, int count) {
#if LOGISTIC_ACCEL_AVAILABLE
    uint32_t fixed_inputs[LOGISTIC_ACCEL_INPUT_SIZE] = {0};
    
    int actual_count = (count > LOGISTIC_ACCEL_INPUT_SIZE) ? LOGISTIC_ACCEL_INPUT_SIZE : count;
    
    for (int i = 0; i < actual_count; i++) {
        fixed_inputs[i] = (uint32_t)double_to_fixed(inputs[i], FIXED_POINT_FRACTIONAL_BITS);
    }
    
    logistic_input_write(fixed_inputs);
#endif
}

/**
 * Set weight values in the accelerator
 */
void logistic_accel_set_weights(double *weights, int count) {
#if LOGISTIC_ACCEL_AVAILABLE
    uint32_t fixed_weights[LOGISTIC_ACCEL_INPUT_SIZE] = {0};
    
    int actual_count = (count > LOGISTIC_ACCEL_INPUT_SIZE) ? LOGISTIC_ACCEL_INPUT_SIZE : count;
    
    for (int i = 0; i < actual_count; i++) {
        fixed_weights[i] = (uint32_t)double_to_fixed(weights[i], FIXED_POINT_FRACTIONAL_BITS);
    }
    
    logistic_weight_write(fixed_weights);
#endif
}

/**
 * Get the result from the accelerator
 */
int32_t logistic_accel_get_result(void) {
#if LOGISTIC_ACCEL_AVAILABLE
    return (int32_t)logistic_result_read();
#else
    return 0;
#endif
}

/**
 * Compute dot product for a chunk of up to 8 elements
 */
int32_t logistic_accel_compute_chunk(double *inputs, double *weights, int chunk_size) {
#if LOGISTIC_ACCEL_AVAILABLE
    // Set inputs and weights
    logistic_accel_set_inputs(inputs, chunk_size);
    logistic_accel_set_weights(weights, chunk_size);
    
    // The result is available immediately due to combinatorial logic
    return logistic_accel_get_result();
#else
    // Fallback software implementation
    int32_t result = 0;
    for (int i = 0; i < chunk_size; i++) {
        result += double_to_fixed(inputs[i] * weights[i], FIXED_POINT_FRACTIONAL_BITS);
    }
    return result;
#endif
}

/**
 * Compute dot product for 64-element vectors using the hardware accelerator
 * Processes the vectors in chunks of 8 elements
 */
double logistic_accel_dot_product(size_t size, double *inputs, double *weights) {
    int64_t total_result = 0;
    int logistic_accel_chuncks = size / LOGISTIC_ACCEL_INPUT_SIZE;
    // Process in chunks
    for (int chunk = 0; chunk < logistic_accel_chuncks; chunk++) {
        int offset = chunk * LOGISTIC_ACCEL_INPUT_SIZE;
        int32_t chunk_result = logistic_accel_compute_chunk(
            &inputs[offset], 
            &weights[offset], 
            LOGISTIC_ACCEL_INPUT_SIZE
        );
        total_result += chunk_result;
    }
    
    // Convert back to double
    return fixed_to_double((int32_t)total_result, FIXED_POINT_FRACTIONAL_BITS);
}