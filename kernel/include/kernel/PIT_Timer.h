#ifndef _PIT_TIMER_H
#define _PIT_TIMER_H

#include <stdint.h>

// -----------------------
// Constants
// -----------------------

#define PIT_INPUT_FREQ  1193182
#define PIT_OUTPUT_FREQ 100      // 10ms/tick
#define PIT_MS_PER_TICK 10
#define PIT_NS_PER_TICK PIT_MS_PER_TICK * 1000000

#define CHANNEL0_DATA_PORT 0x40
#define COMMAND_REGISTER   0x43  // Write-only

// Channel 0, lobyte/hibyte, rate gen, binary mode
#define PIT_CONFIG  0x34

// -----------------------
// Main Functionality
// -----------------------

void initialize_PIT_timer(uint32_t frequency);

#endif // _PIT_TIMER_H
