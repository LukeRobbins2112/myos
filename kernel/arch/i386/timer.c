#include <kernel/timer.h>

// Global timer tick variable, incremented on IRQ0
uint64_t tick = 0;


void clock_tick(){
  tick++;
}

uint64_t ticks_since_boot(){
  return tick;
}

uint64_t ms_since_boot(){
  // @TODO abstract this away from PIT
  return tick * PIT_MS_PER_TICK;
}
