#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <kernel/PIT_Timer.h>

// Global timer counter, starting from boot
// Incremented with each IRQ0
// Actual time depends on PIT/HPET/ACPI frequency
extern uint64_t tick;

// @TODO add HPET, Used for high resolution

void clock_tick();
uint64_t ticks_since_boot();
uint64_t ms_since_boot();

#endif // TIMER_H
