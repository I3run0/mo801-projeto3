#include "dot_product_accel.h"
#include <stddef.h>
#include <generated/csr.h>

// Fixed-point arithmetic configuration (Q16.16 format)
#define FIXED_POINT_FRACTIONAL_BITS 16

int32_t double_to_fixed(double value, int fractional_bits);
double fixed_to_double(int32_t value, int fractional_bits);
double fixed64_to_double(int64_t value, int fractional_bits);
void dot_product_accel_init(void);
void dot_product_accel_set_inputs(double *inputs, int count);
void dot_product_accel_set_weights(double *weights, int count);
int32_t dot_product_accel_get_result(void);
int32_t dot_product_accel_compute_chunk(double *inputs, double *weights, int chunk_size);
double dot_product_accel_dot_product(size_t size, double *inputs, double *weights);

// CSR access functions for dot product accelerator inputs and weights
#if DOT_PRODUCT_ACCEL_AVAILABLE

// Input CSR write functions
static inline void dot_product_input_write(uint32_t *values) {
    for (int i = 0; i < DOT_PRODUCT_ACCEL_INPUT_SIZE; i++) {
        csr_write_simple(values[i], CSR_DOT_PRODUCT_ACCEL_INPUT_ADDR + (i * 4));
    }
}

// Weight CSR write functions  
static inline void dot_product_weight_write(uint32_t *values) {
    for (int i = 0; i < DOT_PRODUCT_ACCEL_INPUT_SIZE; i++) {
        csr_write_simple(values[i], CSR_DOT_PRODUCT_ACCEL_WEIGHT_ADDR + (i * 4));
    }
}

#endif

/**
 * Initialize the dot product accelerator
 */
void dot_product_accel_init(void) {
#if DOT_PRODUCT_ACCEL_AVAILABLE
    // Clear all input and weight registers
    uint32_t zeros[DOT_PRODUCT_ACCEL_INPUT_SIZE] = {0};
    dot_product_input_write(zeros);
    dot_product_weight_write(zeros);
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

double fixed64_to_double(int64_t value, int fractional_bits) {
    return (double)value / (1 << fractional_bits);
}

/**
 * Set input values in the accelerator
 */
void dot_product_accel_set_inputs(double *inputs, int count) {
#if DOT_PRODUCT_ACCEL_AVAILABLE
    uint32_t fixed_inputs[DOT_PRODUCT_ACCEL_INPUT_SIZE] = {0};
    
    int actual_count = (count > DOT_PRODUCT_ACCEL_INPUT_SIZE) ? DOT_PRODUCT_ACCEL_INPUT_SIZE : count;
    
    for (int i = 0; i < actual_count; i++) {
        fixed_inputs[i] = (uint32_t)double_to_fixed(inputs[i], FIXED_POINT_FRACTIONAL_BITS);
    }
    
    dot_product_input_write(fixed_inputs);
#endif
}

/**
 * Set weight values in the accelerator
 */
void dot_product_accel_set_weights(double *weights, int count) {
#if DOT_PRODUCT_ACCEL_AVAILABLE
    uint32_t fixed_weights[DOT_PRODUCT_ACCEL_INPUT_SIZE] = {0};
    
    int actual_count = (count > DOT_PRODUCT_ACCEL_INPUT_SIZE) ? DOT_PRODUCT_ACCEL_INPUT_SIZE : count;
    
    for (int i = 0; i < actual_count; i++) {
        fixed_weights[i] = (uint32_t)double_to_fixed(weights[i], FIXED_POINT_FRACTIONAL_BITS);
    }
    
    dot_product_weight_write(fixed_weights);
#endif
}

/**
 * Get the result from the accelerator
 */
int32_t dot_product_accel_get_result(void) {
#if DOT_PRODUCT_ACCEL_AVAILABLE
    return (int32_t)dot_product_accel_result_read();
#else
    return 0;
#endif
}

/**
 * Compute dot product for a chunk of up to DOT_PRODUCT_ACCEL_INPUT_SIZE elements
 */
int32_t dot_product_accel_compute_chunk(double *inputs, double *weights, int chunk_size) {
#if DOT_PRODUCT_ACCEL_AVAILABLE
    // Set inputs and weights
    dot_product_accel_set_inputs(inputs, chunk_size);
    dot_product_accel_set_weights(weights, chunk_size);
    
    // The result is available immediately due to combinatorial logic
    return dot_product_accel_get_result();
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
 * Compute dot product for vectors using the hardware accelerator
 * Processes the vectors in chunks of DOT_PRODUCT_ACCEL_INPUT_SIZE elements
 * and handles any remaining elements
 */
double dot_product_accel_dot_product(size_t size, double *inputs, double *weights) {
    int64_t total_result = 0;
    size_t processed = 0;
    
    // Process full chunks
    while (processed + DOT_PRODUCT_ACCEL_INPUT_SIZE <= size) {
        int32_t chunk_result = dot_product_accel_compute_chunk(
            &inputs[processed], 
            &weights[processed], 
            DOT_PRODUCT_ACCEL_INPUT_SIZE
        );
        total_result += chunk_result;
        processed += DOT_PRODUCT_ACCEL_INPUT_SIZE;
    }
    
    // Handle remaining elements that don't fit in a complete chunk
    if (processed < size) {
        int32_t remainder_result = dot_product_accel_compute_chunk(
            &inputs[processed], 
            &weights[processed], 
            size - processed
        );
        total_result += remainder_result;
    }
    
    // Convert back to double
    return fixed64_to_double(total_result, FIXED_POINT_FRACTIONAL_BITS);
}

// Legacy function name for backward compatibility
double logistic_accel_dot_product(size_t size, double *inputs, double *weights) {
    return dot_product_accel_dot_product(size, inputs, weights);
}