#ifndef __TIMER_DRIVER_H
#define __TIMER_DRIVER_H

#include <generated/csr.h>

void start_stopwatch(void);
void stop_stopwatch(void);
uint32_t get_elapsed_ticks(void);

#endif /* __TIMER_DRIVER_H */