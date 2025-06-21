#ifndef __INFERENCE_ACCEL_H
#define __INFERENCE_ACCEL_H

#include <stdint.h>
#include <generated/csr.h>

#ifdef CSR_INFERENCE_ACCEL_BASE

// Control register bits
#define INFERENCE_ACCEL_CTRL_START  (1 << 0)
#define INFERENCE_ACCEL_CTRL_RESET  (1 << 1)

// Status register bits
#define INFERENCE_ACCEL_STATUS_READY (1 << 0)
#define INFERENCE_ACCEL_STATUS_DONE  (1 << 1)
#define INFERENCE_ACCEL_STATUS_BUSY  (1 << 2)

// Fixed point conversion macros (Q16.16 format)
#define FLOAT_TO_FIXED(x) ((int32_t)((x) * 65536.0))
#define FIXED_TO_FLOAT(x) (((double)(x)) / 65536.0)


static inline void inference_accel_reset(void) {
    inference_accel_control_write(INFERENCE_ACCEL_CTRL_RESET);
    inference_accel_control_write(0);  // Clear reset
}

// Implementation
static inline void inference_accel_init(void) {
    inference_accel_reset();
}

static inline int inference_accel_is_ready(void) {
    return (inference_accel_status_read() & INFERENCE_ACCEL_STATUS_READY) != 0;
}

static inline int inference_accel_is_done(void) {
    return (inference_accel_status_read() & INFERENCE_ACCEL_STATUS_DONE) != 0;
}

static inline int inference_accel_is_busy(void) {
    return (inference_accel_status_read() & INFERENCE_ACCEL_STATUS_BUSY) != 0;
}

static inline void inference_accel_set_params(double weight, double bias) {
    inference_accel_weight_write(FLOAT_TO_FIXED(weight));
    inference_accel_bias_write(FLOAT_TO_FIXED(bias));
}

static inline void inference_accel_set_params_fixed(int32_t weight_fixed, int32_t bias_fixed) {
    inference_accel_weight_write(weight_fixed);
    inference_accel_bias_write(bias_fixed);
}

static inline void inference_accel_wait_done(void) {
    while (!inference_accel_is_done()) {
        // Wait for completion
    }
}

static inline int32_t inference_accel_compute_fixed(int32_t input_fixed) {
    // Wait until ready
    while (!inference_accel_is_ready()) {
        // Wait
    }
    
    // Set input data
    inference_accel_input_data_write(input_fixed);
    
    // Start computation
    inference_accel_control_write(INFERENCE_ACCEL_CTRL_START);
    inference_accel_control_write(0);  // Clear start bit
    
    // Wait for completion
    inference_accel_wait_done();
    
    // Return result
    return inference_accel_result_read();
}

static inline int32_t inference_accel_compute(double input) {
    return inference_accel_compute_fixed(FLOAT_TO_FIXED(input));
}

static inline double inference_accel_get_result_float(void) {
    return FIXED_TO_FLOAT(inference_accel_result_read());
}

static inline int32_t inference_accel_get_result_fixed(void) {
    return inference_accel_result_read();
}

#endif // CSR_INFERENCE_ACCEL_BASE

#endif // __INFERENCE_ACCEL_H
