#include <kernel/sleep.h>
#include <kernel/timer.h>
#include <kernel/multitasking.h>
#include <kernel/kheap.h>
#include <stdio.h>

sleeping_task_t * sleep_queue_head = 0;
sleeping_task_t * sleep_queue_tail = 0;

void add_to_sleep_queue(sleeping_task_t* sleeper){
  // Empty list, just add the node
  if (!sleep_queue_head){
    sleeper->next = 0;
    sleep_queue_head = sleeper;
    sleep_queue_tail = sleeper;
    return;
  }

  // Time is lower than all in the list, append to front
  if (sleeper->wake_time < sleep_queue_head->wake_time){
    sleeper->next = sleep_queue_head;
    sleep_queue_head = sleeper;
  }

  // Search until we find a node where the next node's val is greater than the new node
  sleeping_task_t* iter = sleep_queue_head;
  while (iter->next && iter->next->wake_time < sleeper->wake_time){
    iter = iter->next;
  }

  // Fix up the linked list
  sleeper->next = iter->next;
  iter->next = sleeper;
}

void ms_sleep_until(uint64_t wake_time_ms){
  sleeping_task_t* new_sleeper = (sleeping_task_t*)kalloc(sizeof(sleeping_task_t), 0, kheap);
  if (!new_sleeper){
    printf("Failed to alloc memory for new sleeper!\n");
  }

  // Fill out sleeping task structure
  new_sleeper->task = get_current_task();
  new_sleeper->wake_time = wake_time_ms;

  // Append to queue
  add_to_sleep_queue(new_sleeper);

  // Block the task until it wakes
  block_curr_task();
}

void ms_sleep(uint64_t sleep_duration){
  uint64_t wake_time_ms = ms_since_boot() + sleep_duration;
  ms_sleep_until(wake_time_ms);
}


void wake_sleeping_tasks(){

  // Get current time in millis
  uint64_t time_millis = ms_since_boot();

  // For all sleeping tasks, unblock them (no pre-empting), remove the node from the queue
  // Update sleep list, and de-allocate now-unused links
  while (sleep_queue_head && sleep_queue_head->wake_time <= time_millis){
    unblock_task(sleep_queue_head->task, 0);
    sleeping_task_t* to_delete = sleep_queue_head;
    sleep_queue_head = sleep_queue_head->next;
    kfree(to_delete, kheap);
  }
}
