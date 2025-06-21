#include <stdio.h>
#include <stddef.h>
#include <generated/csr.h>
#include <irq.h>
#include <uart.h>
#include "inference_accel.h"

uint32_t start_ticks;
uint32_t elapsed_ticks;

void start_stopwatch() {
    // Disable timer
    timer0_en_write(0);
    
    // Set timer to count down from maximum value
    timer0_reload_write(0xffffffff);
    timer0_load_write(0xffffffff);
    
    // Enable timer
    timer0_en_write(1);
    
    // Update and read initial value
    timer0_update_value_write(1);
    start_ticks = timer0_value_read();
}

void stop_stopwatch(void) {
    uint32_t end_ticks;
    
    // Update and read final value
    timer0_update_value_write(1);
    end_ticks = timer0_value_read();
    
    // Calculate elapsed ticks (timer counts down)
    elapsed_ticks = start_ticks - end_ticks;
}

// Print elapsed time without using floats
void print_elapsed_time(uint32_t ticks, const char* benchmark_name) {
    // Convert ticks to microseconds first, then to milliseconds
    uint32_t microseconds = ticks / (CONFIG_CLOCK_FREQUENCY / 1000000);
    uint32_t milliseconds = microseconds / 1000;
    uint32_t seconds = milliseconds / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t rem_seconds = seconds % 60;
    uint32_t rem_milliseconds = milliseconds % 1000;
    uint32_t MHz = CONFIG_CLOCK_FREQUENCY / 1000000;
    
    printf("=== %s Results ===\n", benchmark_name);
    printf("Raw ticks: %lu\n", ticks);
    printf("Elapsed time: %02d:%02d.%03d (%lu milliseconds)\n", 
           (int)minutes, (int)rem_seconds, (int)rem_milliseconds, (unsigned long)milliseconds);
    printf("CPU: %s @ %luMHz\n", CONFIG_CPU_HUMAN_NAME, (unsigned long)MHz);
    printf("Clock frequency: %lu Hz\n", (unsigned long)CONFIG_CLOCK_FREQUENCY);
    printf("\n");
}

// Software prediction functions
double predict(double x) {
    return x * 938.237861251353 + 152.91886182616113;
}

int predict_int(double x) {
    return (((int)(x * 100)) * 93823 + 1529188) / 100;
}

int main() {
    printf("LiteX Benchmark with Hardware Accelerator Starting...\n");
    
    double input = 0.03; // valor da feature
    volatile double p1 = 0;
    volatile int p2 = 0;
    volatile int p3 = 0; // Hardware accelerator results
    int i;
    
    // Initialize hardware accelerator
#ifdef CSR_INFERENCE_ACCEL_BASE
    printf("Initializing inference accelerator...\n");
    inference_accel_init();
    inference_accel_set_params(938.237861251353, 152.91886182616113);
    printf("Hardware accelerator initialized!\n\n");
#else
    printf("Warning: Inference accelerator not available in this build\n\n");
#endif
    
    // First benchmark - floating point prediction (CPU)
    printf("Running CPU floating point benchmark...\n");
    start_stopwatch();
    
    for (i = 0; i < 100000; i += 1) {
        p1 += predict(input);
    }
    
    stop_stopwatch();
    print_elapsed_time(elapsed_ticks, "CPU Floating Point Benchmark");
    
    // Second benchmark - integer prediction (CPU)
    printf("Running CPU integer benchmark...\n");
    start_stopwatch();
    
    for (i = 0; i < 100000; i += 1) {
        p2 += predict_int(input);
    }
    
    stop_stopwatch();
    print_elapsed_time(elapsed_ticks, "CPU Integer Benchmark");
    
    // Third benchmark - hardware accelerated prediction
#ifdef CSR_INFERENCE_ACCEL_BASE
    printf("Running hardware accelerated benchmark...\n");
    start_stopwatch();
    
    for (i = 0; i < 100000; i += 1) {
        int32_t hw_result = inference_accel_compute(input);
        p3 += (hw_result >> 16); // Convert from Q16.16 to integer for accumulation
    }
    
    stop_stopwatch();
    print_elapsed_time(elapsed_ticks, "Hardware Accelerated Benchmark");
#endif
    
    printf("=== Final Results ===\n");
    printf("CPU FP accumulated result: %.6f\n", p1);
    printf("CPU INT accumulated result: %d\n", p2 / 100);
#ifdef CSR_INFERENCE_ACCEL_BASE
    printf("HW accelerated accumulated result: %d\n", p3);
#endif
    
    // Single prediction comparison
    printf("\n=== Single Prediction Comparison ===\n");
    double single_fp = predict(input);
    int single_int = predict_int(input);
    
    printf("CPU FP single prediction: %.6f\n", single_fp);
    printf("CPU INT single prediction: %d\n", single_int);
    
#ifdef CSR_INFERENCE_ACCEL_BASE
    // Test single hardware prediction
    int32_t hw_single = inference_accel_compute(input);
    double hw_single_float = inference_accel_get_result_float();
    
    printf("HW single prediction (fixed): %ld\n", (long)hw_single);
    printf("HW single prediction (float): %.6f\n", hw_single_float);
    
    // Accuracy comparison
    double hw_error = hw_single_float - single_fp;
    printf("HW vs CPU FP error: %.6f\n", hw_error);
#endif
    
    printf("\n=== Performance Analysis ===\n");
    printf("- CPU Floating point: highest precision, potentially slower\n");
    printf("- CPU Integer: faster than FP, reduced precision\n");
#ifdef CSR_INFERENCE_ACCEL_BASE
    printf("- Hardware accelerator: dedicated pipeline, fixed-point arithmetic\n");
    printf("- HW accelerator should show significant speedup for large batches\n");
#endif
    
    printf("\nBenchmark completed!\n");
    return 0;
}