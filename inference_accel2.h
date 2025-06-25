/**
 * LiteX Logistic Regression Accelerator Driver
 * 
 * This driver provides a high-level API for the LiteX Logistic Regression
 * Accelerator peripheral with 64 input features.
 */

#ifndef LOGISTIC_REGRESSION_DRIVER_H
#define LOGISTIC_REGRESSION_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include <generated/csr.h>

// Configuration constants
#define LOGISTIC_INPUT_SIZE     64
#define LOGISTIC_DATA_WIDTH     32
#define LOGISTIC_TIMEOUT_MS     1000

// Status codes
typedef enum {
    LOGISTIC_OK = 0,
    LOGISTIC_ERROR_TIMEOUT = -1,
    LOGISTIC_ERROR_INVALID_PARAM = -2,
    LOGISTIC_ERROR_BUSY = -3
} logistic_status_t;

// Driver context structure
typedef struct {
    bool initialized;
    uint32_t timeout_ms;
} logistic_ctx_t;

/**
 * Initialize the logistic regression accelerator driver
 * 
 * @param ctx Driver context structure
 * @param timeout_ms Timeout in milliseconds for operations
 * @return LOGISTIC_OK on success, error code otherwise
 */
logistic_status_t logistic_init(logistic_ctx_t* ctx, uint32_t timeout_ms);

/**
 * Set input data for inference
 * 
 * @param ctx Driver context
 * @param input_data Array of 64 input values (32-bit each)
 * @return LOGISTIC_OK on success, error code otherwise
 */
logistic_status_t logistic_set_input_data(logistic_ctx_t* ctx, const uint32_t* input_data);

/**
 * Set weights for the model
 * 
 * @param ctx Driver context
 * @param weights Array of 64 weight values (32-bit each)
 * @return LOGISTIC_OK on success, error code otherwise
 */
logistic_status_t logistic_set_weights(logistic_ctx_t* ctx, const uint32_t* weights);

/**
 * Set bias value for the model
 * 
 * @param ctx Driver context
 * @param bias Bias value (32-bit)
 * @return LOGISTIC_OK on success, error code otherwise
 */
logistic_status_t logistic_set_bias(logistic_ctx_t* ctx, uint32_t bias);

/**
 * Start inference computation
 * 
 * @param ctx Driver context
 * @return LOGISTIC_OK on success, error code otherwise
 */
logistic_status_t logistic_start_inference(logistic_ctx_t* ctx);

/**
 * Check if computation is complete
 * 
 * @param ctx Driver context
 * @return true if done, false if still processing
 */
bool logistic_is_done(logistic_ctx_t* ctx);

/**
 * Wait for computation to complete
 * 
 * @param ctx Driver context
 * @return LOGISTIC_OK on success, LOGISTIC_ERROR_TIMEOUT on timeout
 */
logistic_status_t logistic_wait_done(logistic_ctx_t* ctx);

/**
 * Get inference result
 * 
 * @param ctx Driver context
 * @param result Pointer to store the result (32-bit)
 * @return LOGISTIC_OK on success, error code otherwise
 */
logistic_status_t logistic_get_result(logistic_ctx_t* ctx, uint32_t* result);

/**
 * Perform complete inference (blocking operation)
 * 
 * @param ctx Driver context
 * @param input_data Array of 64 input values
 * @param weights Array of 64 weight values
 * @param bias Bias value
 * @param result Pointer to store the result
 * @return LOGISTIC_OK on success, error code otherwise
 */
logistic_status_t logistic_inference(logistic_ctx_t* ctx, 
                                   const uint32_t* input_data,
                                   const uint32_t* weights,
                                   uint32_t bias,
                                   uint32_t* result);

/**
 * Reset the accelerator
 * 
 * @param ctx Driver context
 * @return LOGISTIC_OK on success, error code otherwise
 */
logistic_status_t logistic_reset(logistic_ctx_t* ctx);

// Low-level register access functions (for advanced usage)

/**
 * Write to input data register by index
 * 
 * @param index Input data index (0-63)
 * @param value 32-bit value to write
 * @return LOGISTIC_OK on success, error code otherwise
 */
logistic_status_t logistic_write_input_data_reg(uint8_t index, uint32_t value);

/**
 * Write to weight register by index
 * 
 * @param index Weight index (0-63)
 * @param value 32-bit value to write
 * @return LOGISTIC_OK on success, error code otherwise
 */
logistic_status_t logistic_write_weight_reg(uint8_t index, uint32_t value);

/**
 * Read from input data register by index
 * 
 * @param index Input data index (0-63)
 * @param value Pointer to store the read value
 * @return LOGISTIC_OK on success, error code otherwise
 */
logistic_status_t logistic_read_input_data_reg(uint8_t index, uint32_t* value);

/**
 * Read from weight register by index
 * 
 * @param index Weight index (0-63)
 * @param value Pointer to store the read value
 * @return LOGISTIC_OK on success, error code otherwise
 */
logistic_status_t logistic_read_weight_reg(uint8_t index, uint32_t* value);

#endif // LOGISTIC_REGRESSION_DRIVER_H

//================================================================================
// Implementation
//================================================================================

#include <stdio.h>
#include <string.h>

// Internal helper functions
static uint32_t get_time_ms(void);
static logistic_status_t logistic_write_input_data_array(const uint32_t* data);
static logistic_status_t logistic_write_weights_array(const uint32_t* weights);

// Simple time tracking (replace with your system's time function)
static uint32_t system_time_ms = 0;
static uint32_t get_time_ms(void) {
    return system_time_ms++; // Placeholder - implement actual timing
}

logistic_status_t logistic_init(logistic_ctx_t* ctx, uint32_t timeout_ms) {
    if (!ctx) {
        return LOGISTIC_ERROR_INVALID_PARAM;
    }
    
    ctx->initialized = true;
    ctx->timeout_ms = timeout_ms > 0 ? timeout_ms : LOGISTIC_TIMEOUT_MS;
    
    // Reset the accelerator to known state
    return logistic_reset(ctx);
}

logistic_status_t logistic_set_input_data(logistic_ctx_t* ctx, const uint32_t* input_data) {
    if (!ctx || !ctx->initialized || !input_data) {
        return LOGISTIC_ERROR_INVALID_PARAM;
    }
    
    return logistic_write_input_data_array(input_data);
}

logistic_status_t logistic_set_weights(logistic_ctx_t* ctx, const uint32_t* weights) {
    if (!ctx || !ctx->initialized || !weights) {
        return LOGISTIC_ERROR_INVALID_PARAM;
    }
    
    return logistic_write_weights_array(weights);
}

logistic_status_t logistic_set_bias(logistic_ctx_t* ctx, uint32_t bias) {
    if (!ctx || !ctx->initialized) {
        return LOGISTIC_ERROR_INVALID_PARAM;
    }
    
    logistic_bias_write(bias);
    return LOGISTIC_OK;
}

logistic_status_t logistic_start_inference(logistic_ctx_t* ctx) {
    if (!ctx || !ctx->initialized) {
        return LOGISTIC_ERROR_INVALID_PARAM;
    }
    
    // Check if already busy
    if (!logistic_is_done(ctx)) {
        return LOGISTIC_ERROR_BUSY;
    }
    
    // Start computation by writing to start register
    logistic_start_write(1);
    
    return LOGISTIC_OK;
}

bool logistic_is_done(logistic_ctx_t* ctx) {
    if (!ctx || !ctx->initialized) {
        return false;
    }
    
    return (logistic_done_read() & 0x1) != 0;
}

logistic_status_t logistic_wait_done(logistic_ctx_t* ctx) {
    if (!ctx || !ctx->initialized) {
        return LOGISTIC_ERROR_INVALID_PARAM;
    }
    
    uint32_t start_time = get_time_ms();
    
    while (!logistic_is_done(ctx)) {
        if ((get_time_ms() - start_time) > ctx->timeout_ms) {
            return LOGISTIC_ERROR_TIMEOUT;
        }
        // Small delay to prevent busy waiting
        // In real implementation, use appropriate delay function
    }
    
    return LOGISTIC_OK;
}

logistic_status_t logistic_get_result(logistic_ctx_t* ctx, uint32_t* result) {
    if (!ctx || !ctx->initialized || !result) {
        return LOGISTIC_ERROR_INVALID_PARAM;
    }
    
    *result = logistic_output_read();
    return LOGISTIC_OK;
}

logistic_status_t logistic_inference(logistic_ctx_t* ctx, 
                                   const uint32_t* input_data,
                                   const uint32_t* weights,
                                   uint32_t bias,
                                   uint32_t* result) {
    logistic_status_t status;
    
    if (!ctx || !ctx->initialized || !input_data || !weights || !result) {
        return LOGISTIC_ERROR_INVALID_PARAM;
    }
    
    // Set input data
    status = logistic_set_input_data(ctx, input_data);
    if (status != LOGISTIC_OK) return status;
    
    // Set weights
    status = logistic_set_weights(ctx, weights);
    if (status != LOGISTIC_OK) return status;
    
    // Set bias
    status = logistic_set_bias(ctx, bias);
    if (status != LOGISTIC_OK) return status;
    
    // Start inference
    status = logistic_start_inference(ctx);
    if (status != LOGISTIC_OK) return status;
    
    // Wait for completion
    status = logistic_wait_done(ctx);
    if (status != LOGISTIC_OK) return status;
    
    // Get result
    return logistic_get_result(ctx, result);
}

logistic_status_t logistic_reset(logistic_ctx_t* ctx) {
    if (!ctx) {
        return LOGISTIC_ERROR_INVALID_PARAM;
    }
    
    // Reset is typically done through control registers
    // Since there's no explicit reset register for the logistic accelerator,
    // we ensure it's in idle state by checking done status
    
    // Wait a bit for any ongoing operation to complete
    uint32_t start_time = get_time_ms();
    while (!logistic_is_done(ctx) && (get_time_ms() - start_time) < 100) {
        // Wait for idle state
    }
    
    return LOGISTIC_OK;
}

// Low-level register access functions
logistic_status_t logistic_write_input_data_reg(uint8_t index, uint32_t value) {
    if (index >= LOGISTIC_INPUT_SIZE) {
        return LOGISTIC_ERROR_INVALID_PARAM;
    }
    
    // Calculate register address for input data
    uint32_t reg_addr = CSR_LOGISTIC_INPUT_DATA_ADDR + (index * 4);
    csr_write_simple(value, reg_addr);
    
    return LOGISTIC_OK;
}

logistic_status_t logistic_write_weight_reg(uint8_t index, uint32_t value) {
    if (index >= LOGISTIC_INPUT_SIZE) {
        return LOGISTIC_ERROR_INVALID_PARAM;
    }
    
    // Calculate register address for weights
    uint32_t reg_addr = CSR_LOGISTIC_WEIGHTS_ADDR + (index * 4);
    csr_write_simple(value, reg_addr);
    
    return LOGISTIC_OK;
}

logistic_status_t logistic_read_input_data_reg(uint8_t index, uint32_t* value) {
    if (index >= LOGISTIC_INPUT_SIZE || !value) {
        return LOGISTIC_ERROR_INVALID_PARAM;
    }
    
    // Calculate register address for input data
    uint32_t reg_addr = CSR_LOGISTIC_INPUT_DATA_ADDR + (index * 4);
    *value = csr_read_simple(reg_addr);
    
    return LOGISTIC_OK;
}

logistic_status_t logistic_read_weight_reg(uint8_t index, uint32_t* value) {
    if (index >= LOGISTIC_INPUT_SIZE || !value) {
        return LOGISTIC_ERROR_INVALID_PARAM;
    }
    
    // Calculate register address for weights
    uint32_t reg_addr = CSR_LOGISTIC_WEIGHTS_ADDR + (index * 4);
    *value = csr_read_simple(reg_addr);
    
    return LOGISTIC_OK;
}

// Internal helper functions
static logistic_status_t logistic_write_input_data_array(const uint32_t* data) {
    for (int i = 0; i < LOGISTIC_INPUT_SIZE; i++) {
        logistic_status_t status = logistic_write_input_data_reg(i, data[i]);
        if (status != LOGISTIC_OK) {
            return status;
        }
    }
    return LOGISTIC_OK;
}

static logistic_status_t logistic_write_weights_array(const uint32_t* weights) {
    for (int i = 0; i < LOGISTIC_INPUT_SIZE; i++) {
        logistic_status_t status = logistic_write_weight_reg(i, weights[i]);
        if (status != LOGISTIC_OK) {
            return status;
        }
    }
    return LOGISTIC_OK;
}

//================================================================================
// Example Usage
//================================================================================

/*
// Example usage of the driver:

#include "logistic_regression_driver.h"

void example_usage(void) {
    logistic_ctx_t ctx;
    logistic_status_t status;
    
    // Initialize driver
    status = logistic_init(&ctx, 1000); // 1 second timeout
    if (status != LOGISTIC_OK) {
        printf("Failed to initialize logistic driver: %d\n", status);
        return;
    }
    
    // Prepare test data
    uint32_t input_data[64];
    uint32_t weights[64];
    uint32_t bias = 0x12345678;
    uint32_t result;
    
    // Fill with test values
    for (int i = 0; i < 64; i++) {
        input_data[i] = i + 1;       // Test input values
        weights[i] = (i + 1) * 2;    // Test weight values
    }
    
    // Perform inference (blocking)
    status = logistic_inference(&ctx, input_data, weights, bias, &result);
    if (status == LOGISTIC_OK) {
        printf("Inference result: 0x%08X\n", result);
    } else {
        printf("Inference failed: %d\n", status);
    }
    
    // Alternative: Non-blocking approach
    status = logistic_set_input_data(&ctx, input_data);
    status |= logistic_set_weights(&ctx, weights);
    status |= logistic_set_bias(&ctx, bias);
    
    if (status == LOGISTIC_OK) {
        status = logistic_start_inference(&ctx);
        if (status == LOGISTIC_OK) {
            // Do other work while computation runs...
            
            // Check periodically or wait
            status = logistic_wait_done(&ctx);
            if (status == LOGISTIC_OK) {
                status = logistic_get_result(&ctx, &result);
                printf("Non-blocking inference result: 0x%08X\n", result);
            }
        }
    }
}
*/