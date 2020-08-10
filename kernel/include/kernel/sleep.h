#ifndef SLEEP_H
#define SLEEP_H

#include <stdint.h>
#include <kernel/task_control_block.h>

// Sleep queue structure
// Used to track sleeping tasks

typedef struct sleeping_task {
  tcb_t* task;
  uint64_t wake_time;

  // Linked list
  struct sleeping_task* next;
} sleeping_task_t;

// -------------
// Helpers
// ------------

void add_to_sleep_queue(sleeping_task_t* sleeper);

// ---------------------------------
// Main API
// ---------------------------------

void ms_sleep_until(uint64_t milliseconds);
void ms_sleep(uint64_t milliseconds);

void wake_sleeping_tasks();


#endif // SLEEP_H
