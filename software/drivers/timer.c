#include "timer.h"

uint32_t start_ticks;
uint32_t elapsed_ticks;

uint32_t get_elapsed_ticks(void) {
    return elapsed_ticks;
}
void start_stopwatch(void) {
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