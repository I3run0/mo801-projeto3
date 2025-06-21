#ifndef __INFERENCE_ACCEL_H
#define __INFERENCE_ACCEL_H

#include <stdint.h>
#include <generated/csr.h>

#ifdef CSR_INFERENCE_ACCEL_BASE

// Fixed point conversion macros (Q16.16 format)
#define FLOAT_TO_FIXED(x) ((int32_t)((x) * 65536.0))
#define FIXED_TO_FLOAT(x) (((double)(x)) / 65536.0)


static inline void inference_accel_set_params(double weight, double bias) {
    inference_accel_weight_write(FLOAT_TO_FIXED(weight));
    inference_accel_bias_write(FLOAT_TO_FIXED(bias));
}

static inline void inference_accel_set_params_fixed(int32_t weight_fixed, int32_t bias_fixed) {
    inference_accel_weight_write(weight_fixed);
    inference_accel_bias_write(bias_fixed);
}

static inline int32_t inference_accel_compute_fixed(int32_t input_fixed) {
    // Set input data
    inference_accel_input_data_write(input_fixed);

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
