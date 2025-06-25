#ifndef LOGISTIC_REGRESSION_DRIVER_H
#define LOGISTIC_REGRESSION_DRIVER_H

#include <stdint.h>
#include <generated/csr.h>

//--------------------------------------------------------------------------------
// Constants and Configuration
//--------------------------------------------------------------------------------

#define LOGISTIC_INPUT_SIZE     8
#define LOGISTIC_DATA_WIDTH     32
#define LOGISTIC_MAX_VALUE      0xFFFFFFFFUL

//--------------------------------------------------------------------------------
// Low-level Register Access Functions
//--------------------------------------------------------------------------------

/**
 * Write a single input value to the logistic accelerator
 * @param index: Input index (0-7)
 * @param value: 32-bit input value
 */
static inline void logistic_set_input(uint8_t index, uint32_t value) {
    if (index >= LOGISTIC_INPUT_SIZE) return;
    
    // Calculate the register address and bit offset
    uint32_t reg_index = index / 4;  // Each register holds 4 values (128 bits / 32 bits)
    uint32_t bit_offset = (index % 4) * 32;
    
    // Read current register value
    uint64_t reg_addr = CSR_LOGISTIC_LOGISTIC_INPUT_ADDR + (reg_index * 4);
    uint64_t current_val = 0;
    
    // For multi-word registers, we need to handle the bit manipulation carefully
    if (reg_index == 0) {
        // First register (inputs 0-3): bits 0-127
        current_val = csr_read_simple(reg_addr);
        current_val &= ~(0xFFFFFFFFULL << bit_offset);
        current_val |= ((uint64_t)value << bit_offset);
        csr_write_simple((uint32_t)current_val, reg_addr);
        if (bit_offset >= 32) {
            csr_write_simple((uint32_t)(current_val >> 32), reg_addr + 4);
        }
    } else {
        // Second register (inputs 4-7): bits 128-255
        current_val = csr_read_simple(reg_addr);
        current_val &= ~(0xFFFFFFFFULL << bit_offset);
        current_val |= ((uint64_t)value << bit_offset);
        csr_write_simple((uint32_t)current_val, reg_addr);
        if (bit_offset >= 32) {
            csr_write_simple((uint32_t)(current_val >> 32), reg_addr + 4);
        }
    }
}

/**
 * Write a single weight value to the logistic accelerator
 * @param index: Weight index (0-7)
 * @param value: 32-bit weight value
 */
static inline void logistic_set_weight(uint8_t index, uint32_t value) {
    if (index >= LOGISTIC_INPUT_SIZE) return;
    
    // Calculate the register address and bit offset
    uint32_t reg_index = index / 4;
    uint32_t bit_offset = (index % 4) * 32;
    
    // Read current register value
    uint64_t reg_addr = CSR_LOGISTIC_LOGISTIC_WEIGHT_ADDR + (reg_index * 4);
    uint64_t current_val = 0;
    
    if (reg_index == 0) {
        current_val = csr_read_simple(reg_addr);
        current_val &= ~(0xFFFFFFFFULL << bit_offset);
        current_val |= ((uint64_t)value << bit_offset);
        csr_write_simple((uint32_t)current_val, reg_addr);
        if (bit_offset >= 32) {
            csr_write_simple((uint32_t)(current_val >> 32), reg_addr + 4);
        }
    } else {
        current_val = csr_read_simple(reg_addr);
        current_val &= ~(0xFFFFFFFFULL << bit_offset);
        current_val |= ((uint64_t)value << bit_offset);
        csr_write_simple((uint32_t)current_val, reg_addr);
        if (bit_offset >= 32) {
            csr_write_simple((uint32_t)(current_val >> 32), reg_addr + 4);
        }
    }
}

/**
 * Read the computed result from the logistic accelerator
 * @return: 32-bit result value
 */
static inline uint32_t logistic_get_result(void) {
    return logistic_result_read();
}

//--------------------------------------------------------------------------------
// Simplified Multi-word Register Access
//--------------------------------------------------------------------------------

/**
 * Write all input values at once using direct memory access
 * @param inputs: Array of 8 input values
 */
static inline void logistic_set_inputs_direct(const uint32_t inputs[LOGISTIC_INPUT_SIZE]) {
    volatile uint32_t *input_regs = (volatile uint32_t *)CSR_LOGISTIC_LOGISTIC_INPUT_ADDR;
    
    // Write inputs as 32-bit words directly to memory
    for (int i = 0; i < LOGISTIC_INPUT_SIZE; i++) {
        input_regs[i] = inputs[i];
    }
}

/**
 * Write all weight values at once using direct memory access
 * @param weights: Array of 8 weight values
 */
static inline void logistic_set_weights_direct(const uint32_t weights[LOGISTIC_INPUT_SIZE]) {
    volatile uint32_t *weight_regs = (volatile uint32_t *)CSR_LOGISTIC_LOGISTIC_WEIGHT_ADDR;
    
    // Write weights as 32-bit words directly to memory
    for (int i = 0; i < LOGISTIC_INPUT_SIZE; i++) {
        weight_regs[i] = weights[i];
    }
}

//--------------------------------------------------------------------------------
// High-level Driver Functions
//--------------------------------------------------------------------------------

/**
 * Initialize the logistic regression accelerator
 * Clears all inputs and weights to zero
 */
void logistic_init(void);

/**
 * Set all input values for the accelerator
 * @param inputs: Array of 8 input values
 * @return: 0 on success, -1 on error
 */
int logistic_set_inputs(const uint32_t inputs[LOGISTIC_INPUT_SIZE]);

/**
 * Set all weight values for the accelerator
 * @param weights: Array of 8 weight values
 * @return: 0 on success, -1 on error
 */
int logistic_set_weights(const uint32_t weights[LOGISTIC_INPUT_SIZE]);

/**
 * Compute the dot product using the accelerator
 * @param inputs: Array of 8 input values
 * @param weights: Array of 8 weight values
 * @return: Computed dot product result
 */
uint32_t logistic_compute_dot_product(const uint32_t inputs[LOGISTIC_INPUT_SIZE],
                                     const uint32_t weights[LOGISTIC_INPUT_SIZE]);

/**
 * Compute dot product with floating point inputs (converted to fixed point)
 * @param inputs: Array of 8 floating point input values
 * @param weights: Array of 8 floating point weight values
 * @param scale_factor: Fixed point scale factor (e.g., 1000 for 3 decimal places)
 * @return: Computed dot product result (in fixed point format)
 */
uint32_t logistic_compute_dot_product_float(const float inputs[LOGISTIC_INPUT_SIZE],
                                           const float weights[LOGISTIC_INPUT_SIZE],
                                           uint32_t scale_factor);

//--------------------------------------------------------------------------------
// Chunked Processing Functions
//--------------------------------------------------------------------------------

/**
 * Process large datasets in chunks of 8 and accumulate results
 * @param inputs: Input data array (any size)
 * @param weights: Weight data array (same size as inputs)
 * @param data_size: Total number of data points
 * @param accumulator: Pointer to accumulator variable (will be updated)
 * @return: Number of chunks processed, -1 on error
 */
int logistic_process_chunks(const uint32_t *inputs, const uint32_t *weights, 
                           size_t data_size, uint64_t *accumulator);

/**
 * Process large floating-point datasets in chunks of 8 and accumulate results
 * @param inputs: Input data array (any size)
 * @param weights: Weight data array (same size as inputs)
 * @param data_size: Total number of data points
 * @param scale_factor: Fixed point scale factor
 * @param accumulator: Pointer to accumulator variable (will be updated)
 * @return: Number of chunks processed, -1 on error
 */
int logistic_process_chunks_float(const float *inputs, const float *weights,
                                 size_t data_size, uint32_t scale_factor,
                                 uint64_t *accumulator);

/**
 * Complete logistic regression prediction for large datasets
 * @param inputs: Input data array (any size)
 * @param weights: Weight data array (same size as inputs)
 * @param data_size: Total number of data points
 * @param bias: Bias term to add to final result
 * @return: Final prediction result (dot product + bias)
 */
int64_t logistic_predict_large_dataset(const uint32_t *inputs, const uint32_t *weights,
                                      size_t data_size, int32_t bias);

/**
 * Complete logistic regression prediction for large floating-point datasets
 * @param inputs: Input data array (any size)
 * @param weights: Weight data array (same size as inputs)
 * @param data_size: Total number of data points
 * @param scale_factor: Fixed point scale factor
 * @param bias: Bias term (in fixed point format)
 * @return: Final prediction result (dot product + bias)
 */
int64_t logistic_predict_large_dataset_float(const float *inputs, const float *weights,
                                            size_t data_size, uint32_t scale_factor,
                                            int32_t bias);

/**
 * Perform logistic regression prediction (single chunk)
 * @param inputs: Array of 8 input values
 * @param weights: Array of 8 weight values
 * @param bias: Bias term
 * @return: Raw dot product + bias (before sigmoid)
 */
int32_t logistic_predict_raw(const uint32_t inputs[LOGISTIC_INPUT_SIZE],
                            const uint32_t weights[LOGISTIC_INPUT_SIZE],
                            int32_t bias);

/**
 * Test the accelerator with known values
 * @return: 0 on success, -1 on failure
 */
int logistic_self_test(void);

//--------------------------------------------------------------------------------
// Utility Functions
//--------------------------------------------------------------------------------

/**
 * Convert float to fixed point representation
 * @param value: Floating point value
 * @param scale_factor: Scale factor for fixed point
 * @return: Fixed point representation
 */
static inline uint32_t float_to_fixed(float value, uint32_t scale_factor) {
    return (uint32_t)(value * scale_factor);
}

/**
 * Convert fixed point to float representation
 * @param value: Fixed point value
 * @param scale_factor: Scale factor used for fixed point
 * @return: Floating point representation
 */
static inline float fixed_to_float(uint32_t value, uint32_t scale_factor) {
    return (float)value / scale_factor;
}

//--------------------------------------------------------------------------------
// Driver Implementation
//--------------------------------------------------------------------------------

void logistic_init(void) {
    uint32_t zeros[LOGISTIC_INPUT_SIZE] = {0};
    logistic_set_inputs_direct(zeros);
    logistic_set_weights_direct(zeros);
}

int logistic_set_inputs(const uint32_t inputs[LOGISTIC_INPUT_SIZE]) {
    if (!inputs) return -1;
    
    logistic_set_inputs_direct(inputs);
    return 0;
}

int logistic_set_weights(const uint32_t weights[LOGISTIC_INPUT_SIZE]) {
    if (!weights) return -1;
    
    logistic_set_weights_direct(weights);
    return 0;
}

uint32_t logistic_compute_dot_product(const uint32_t inputs[LOGISTIC_INPUT_SIZE],
                                     const uint32_t weights[LOGISTIC_INPUT_SIZE]) {
    if (!inputs || !weights) return 0;
    
    // Set inputs and weights
    logistic_set_inputs_direct(inputs);
    logistic_set_weights_direct(weights);
    
    // Since the result is computed combinatorially, we can read it immediately
    return logistic_get_result();
}

uint32_t logistic_compute_dot_product_float(const float inputs[LOGISTIC_INPUT_SIZE],
                                           const float weights[LOGISTIC_INPUT_SIZE],
                                           uint32_t scale_factor) {
    if (!inputs || !weights || scale_factor == 0) return 0;
    
    uint32_t fixed_inputs[LOGISTIC_INPUT_SIZE];
    uint32_t fixed_weights[LOGISTIC_INPUT_SIZE];
    
    // Convert to fixed point
    for (int i = 0; i < LOGISTIC_INPUT_SIZE; i++) {
        fixed_inputs[i] = float_to_fixed(inputs[i], scale_factor);
        fixed_weights[i] = float_to_fixed(weights[i], scale_factor);
    }
    
    return logistic_compute_dot_product(fixed_inputs, fixed_weights);
}

//--------------------------------------------------------------------------------
// Chunked Processing Implementation
//--------------------------------------------------------------------------------

int logistic_process_chunks(const uint32_t *inputs, const uint32_t *weights, 
                           size_t data_size, uint64_t *accumulator) {
    if (!inputs || !weights || !accumulator) return -1;
    
    size_t chunks_processed = 0;
    size_t remaining_data = data_size;
    
    // Process complete chunks of 8
    while (remaining_data >= LOGISTIC_INPUT_SIZE) {
        uint32_t chunk_result = logistic_compute_dot_product(
            &inputs[chunks_processed * LOGISTIC_INPUT_SIZE],
            &weights[chunks_processed * LOGISTIC_INPUT_SIZE]
        );
        
        *accumulator += chunk_result;
        chunks_processed++;
        remaining_data -= LOGISTIC_INPUT_SIZE;
    }
    
    // Handle remaining data (less than 8 elements)
    if (remaining_data > 0) {
        uint32_t partial_inputs[LOGISTIC_INPUT_SIZE] = {0};
        uint32_t partial_weights[LOGISTIC_INPUT_SIZE] = {0};
        
        // Copy remaining data and pad with zeros
        for (size_t i = 0; i < remaining_data; i++) {
            partial_inputs[i] = inputs[chunks_processed * LOGISTIC_INPUT_SIZE + i];
            partial_weights[i] = weights[chunks_processed * LOGISTIC_INPUT_SIZE + i];
        }
        
        uint32_t partial_result = logistic_compute_dot_product(partial_inputs, partial_weights);
        *accumulator += partial_result;
        chunks_processed++; // Count the partial chunk
    }
    
    return (int)chunks_processed;
}

int logistic_process_chunks_float(const float *inputs, const float *weights,
                                 size_t data_size, uint32_t scale_factor,
                                 uint64_t *accumulator) {
    if (!inputs || !weights || !accumulator || scale_factor == 0) return -1;
    
    size_t chunks_processed = 0;
    size_t remaining_data = data_size;
    
    // Process complete chunks of 8
    while (remaining_data >= LOGISTIC_INPUT_SIZE) {
        uint32_t chunk_result = logistic_compute_dot_product_float(
            &inputs[chunks_processed * LOGISTIC_INPUT_SIZE],
            &weights[chunks_processed * LOGISTIC_INPUT_SIZE],
            scale_factor
        );
        
        *accumulator += chunk_result;
        chunks_processed++;
        remaining_data -= LOGISTIC_INPUT_SIZE;
    }
    
    // Handle remaining data (less than 8 elements)
    if (remaining_data > 0) {
        float partial_inputs[LOGISTIC_INPUT_SIZE] = {0.0f};
        float partial_weights[LOGISTIC_INPUT_SIZE] = {0.0f};
        
        // Copy remaining data and pad with zeros
        for (size_t i = 0; i < remaining_data; i++) {
            partial_inputs[i] = inputs[chunks_processed * LOGISTIC_INPUT_SIZE + i];
            partial_weights[i] = weights[chunks_processed * LOGISTIC_INPUT_SIZE + i];
        }
        
        uint32_t partial_result = logistic_compute_dot_product_float(
            partial_inputs, partial_weights, scale_factor
        );
        *accumulator += partial_result;
        chunks_processed++; // Count the partial chunk
    }
    
    return (int)chunks_processed;
}

int64_t logistic_predict_large_dataset(const uint32_t *inputs, const uint32_t *weights,
                                      size_t data_size, int32_t bias) {
    if (!inputs || !weights) return 0;
    
    uint64_t accumulator = 0;
    int chunks = logistic_process_chunks(inputs, weights, data_size, &accumulator);
    
    if (chunks < 0) return 0; // Error occurred
    
    // Add bias and return final result
    return (int64_t)accumulator + bias;
}

int64_t logistic_predict_large_dataset_float(const float *inputs, const float *weights,
                                            size_t data_size, uint32_t scale_factor,
                                            int32_t bias) {
    if (!inputs || !weights || scale_factor == 0) return 0;
    
    uint64_t accumulator = 0;
    int chunks = logistic_process_chunks_float(inputs, weights, data_size, scale_factor, &accumulator);
    
    if (chunks < 0) return 0; // Error occurred
    
    // Add bias and return final result
    return (int64_t)accumulator + bias;
}

int32_t logistic_predict_raw(const uint32_t inputs[LOGISTIC_INPUT_SIZE],
                            const uint32_t weights[LOGISTIC_INPUT_SIZE],
                            int32_t bias) {
    uint32_t dot_product = logistic_compute_dot_product(inputs, weights);
    return (int32_t)dot_product + bias;
}

int logistic_self_test(void) {
    // Test with simple known values
    uint32_t test_inputs[LOGISTIC_INPUT_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint32_t test_weights[LOGISTIC_INPUT_SIZE] = {1, 1, 1, 1, 1, 1, 1, 1};
    
    uint32_t result = logistic_compute_dot_product(test_inputs, test_weights);
    uint32_t expected = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8; // = 36
    
    if (result != expected) {
        return -1; // Test failed
    }
    
    // Test with zeros
    uint32_t zeros[LOGISTIC_INPUT_SIZE] = {0};
    result = logistic_compute_dot_product(zeros, test_weights);
    if (result != 0) {
        return -1; // Test failed
    }
    
    return 0; // All tests passed
}

//--------------------------------------------------------------------------------
// Example Usage
//--------------------------------------------------------------------------------

/*
// Example 1: Basic dot product computation (single chunk)
void example_basic_usage(void) {
    uint32_t inputs[8] = {100, 200, 150, 300, 250, 180, 220, 160};
    uint32_t weights[8] = {10, 20, 15, 25, 30, 12, 18, 22};
    
    logistic_init();
    uint32_t result = logistic_compute_dot_product(inputs, weights);
    
    printf("Single chunk dot product result: %u\n", result);
}

// Example 2: Large dataset processing with chunked accumulation
void example_chunked_processing(void) {
    // Example with 100 data points (will process as 12 chunks + 4 remaining)
    #define LARGE_DATA_SIZE 100
    uint32_t large_inputs[LARGE_DATA_SIZE];
    uint32_t large_weights[LARGE_DATA_SIZE];
    
    // Initialize with example data
    for (int i = 0; i < LARGE_DATA_SIZE; i++) {
        large_inputs[i] = i + 1;           // 1, 2, 3, ..., 100
        large_weights[i] = (i % 10) + 1;   // 1, 2, 3, ..., 10, 1, 2, ...
    }
    
    logistic_init();
    
    // Method 1: Manual chunked processing with accumulator
    uint64_t accumulator = 0;
    int chunks_processed = logistic_process_chunks(large_inputs, large_weights, 
                                                  LARGE_DATA_SIZE, &accumulator);
    
    printf("Processed %d chunks, accumulated result: %llu\n", chunks_processed, accumulator);
    
    // Method 2: Complete prediction with bias
    int32_t bias = 1000;
    int64_t final_prediction = logistic_predict_large_dataset(large_inputs, large_weights,
                                                             LARGE_DATA_SIZE, bias);
    
    printf("Final prediction with bias: %lld\n", final_prediction);
}

// Example 3: Floating-point large dataset processing
void example_float_chunked_processing(void) {
    #define FLOAT_DATA_SIZE 50
    float float_inputs[FLOAT_DATA_SIZE];
    float float_weights[FLOAT_DATA_SIZE];
    
    // Initialize with example floating-point data
    for (int i = 0; i < FLOAT_DATA_SIZE; i++) {
        float_inputs[i] = (float)(i + 1) * 0.1f;    // 0.1, 0.2, 0.3, ..., 5.0
        float_weights[i] = (float)(i % 5 + 1) * 0.05f; // 0.05, 0.10, 0.15, 0.20, 0.25, ...
    }
    
    logistic_init();
    
    uint32_t scale_factor = 10000; // 4 decimal places precision
    int32_t bias = 5000;           // 0.5 in fixed point
    
    int64_t result = logistic_predict_large_dataset_float(float_inputs, float_weights,
                                                         FLOAT_DATA_SIZE, scale_factor, bias);
    
    // Convert result back to floating point
    float final_result = (float)result / (scale_factor * scale_factor);
    printf("Floating-point prediction result: %.6f\n", final_result);
}

// Example 4: Real-time streaming data processing
void example_streaming_data(void) {
    uint64_t running_accumulator = 0;
    
    // Simulate processing multiple batches of streaming data
    for (int batch = 0; batch < 5; batch++) {
        uint32_t batch_inputs[16];  // 16 data points per batch
        uint32_t batch_weights[16];
        
        // Simulate receiving new data
        for (int i = 0; i < 16; i++) {
            batch_inputs[i] = batch * 16 + i + 1;
            batch_weights[i] = (i % 4) + 1;
        }
        
        // Process this batch and accumulate results
        int chunks = logistic_process_chunks(batch_inputs, batch_weights, 
                                           16, &running_accumulator);
        
        printf("Batch %d: processed %d chunks, running total: %llu\n", 
               batch, chunks, running_accumulator);
    }
    
    printf("Final accumulated result from all batches: %llu\n", running_accumulator);
}

// Example 5: Performance comparison
void example_performance_test(void) {
    #define PERF_DATA_SIZE 1000
    uint32_t perf_inputs[PERF_DATA_SIZE];
    uint32_t perf_weights[PERF_DATA_SIZE];
    
    // Initialize test data
    for (int i = 0; i < PERF_DATA_SIZE; i++) {
        perf_inputs[i] = i + 1;
        perf_weights[i] = (i % 8) + 1;
    }
    
    logistic_init();
    
    // Hardware-accelerated chunked processing
    uint64_t hw_accumulator = 0;
    // Measure time here if needed
    int hw_chunks = logistic_process_chunks(perf_inputs, perf_weights, 
                                           PERF_DATA_SIZE, &hw_accumulator);
    
    // Software equivalent for comparison
    uint64_t sw_accumulator = 0;
    for (int i = 0; i < PERF_DATA_SIZE; i++) {
        sw_accumulator += (uint64_t)perf_inputs[i] * perf_weights[i];
    }
    
    printf("Hardware result: %llu (%d chunks processed)\n", hw_accumulator, hw_chunks);
    printf("Software result: %llu\n", sw_accumulator);
    printf("Results match: %s\n", (hw_accumulator == sw_accumulator) ? "YES" : "NO");
}

// Example 6: Self test with chunked processing
void example_chunked_self_test(void) {
    // Test chunked processing with known results
    uint32_t test_data[24] = {
        1, 2, 3, 4, 5, 6, 7, 8,     // Chunk 1: sum = 36
        2, 4, 6, 8, 10, 12, 14, 16, // Chunk 2: sum = 72  
        1, 1, 1, 1, 1, 1, 1, 1      // Chunk 3: sum = 8
    };
    uint32_t test_weights[24];
    
    // Set all weights to 1
    for (int i = 0; i < 24; i++) {
        test_weights[i] = 1;
    }
    
    logistic_init();
    
    uint64_t accumulator = 0;
    int chunks = logistic_process_chunks(test_data, test_weights, 24, &accumulator);
    
    uint64_t expected = 36 + 72 + 8; // = 116
    
    printf("Chunked self-test: %s\n", 
           (accumulator == expected && chunks == 3) ? "PASSED" : "FAILED");
    printf("Expected: %llu, Got: %llu, Chunks: %d\n", expected, accumulator, chunks);
}
*/

#endif /* LOGISTIC_REGRESSION_DRIVER_H */
