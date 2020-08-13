#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <kernel/PIT_Timer.h>

// Global timer counter, starting from boot
// Incremented with each IRQ0
// Actual time depends on PIT/HPET/ACPI frequency
extern uint64_t tick;

// @TODO add HPET, Used for high resolution


enum TimerType {
    PIT_TIMER = 0,
    HPET_TIMER = 1,
};

void clock_tick();
uint64_t ticks_since_boot();
uint64_t ms_since_boot();
uint64_t ms_per_tick();

#endif // TIMER_H
