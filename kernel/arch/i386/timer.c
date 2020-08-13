#include <kernel/timer.h>


// Global timer tick variable, incremented on IRQ0
uint64_t tick = 0;

// Timer type (PIT, HPET, etc.)
enum TimerType timer_type = PIT_TIMER;

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

uint64_t ms_per_tick(){
  switch(timer_type){
  case PIT_TIMER:
    return PIT_MS_PER_TICK;
  default:
    return 0;
  }
}
