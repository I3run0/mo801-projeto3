
#include "inference_accel2.h"

void main(void) {
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