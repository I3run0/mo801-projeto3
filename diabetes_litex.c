#include <stdio.h>
#include <stddef.h>
#include <generated/csr.h>
#include <irq.h>
#include <uart.h>

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

// Print elapsed time without using floats (similar to your reference function)
void print_elapsed_time(uint32_t ticks, const char* benchmark_name) {
    // Convert ticks to microseconds first, then to milliseconds
    // Avoid float operations for better compatibility
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
    
double predict(double x) {
    return x * 938.237861251353 + 152.91886182616113;
}

int predict_int(double x) {
    return (((int)(x * 100)) * 93823 + 1529188) / 100;
}

int main() {
    printf("LiteX Benchmark Starting...\n");
    
    double input = 0.03; // valor da feature
    volatile double p1 = 0;
    volatile int p2 = 0;
    int i;
    
    // First benchmark - floating point prediction
    printf("Running floating point benchmark...\n");
    start_stopwatch();
    
    for (i = 0; i < 100000; i += 1) {
        p1 += predict(input);
    }
    
    stop_stopwatch();
    print_elapsed_time(elapsed_ticks, "Floating Point Benchmark");
    
    // Second benchmark - integer prediction
    printf("Running integer benchmark...\n");
    start_stopwatch();
    
    for (i = 0; i < 100000; i += 1) {
        p2 += predict_int(input);
    }
    
    stop_stopwatch();
    print_elapsed_time(elapsed_ticks, "Integer Benchmark");

    printf("=== Final Results ===\n");
    printf("Predição FP: %.6f\n", p1);
    printf("Predição INT: %d\n", p2 / 100);
    
    // Performance comparison
    printf("\nPerformance analysis:\n");
    printf("- Floating point operations: more precise but potentially slower\n");
    printf("- Integer operations: faster but with reduced precision\n");
    
    printf("Benchmark completed!\n");
    return 0;
}