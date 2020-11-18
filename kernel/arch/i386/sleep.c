#include <kernel/sleep.h>
#include <kernel/timer.h>
#include <kernel/multitasking.h>
#include <kernel/kheap.h>
#include <stdio.h>
#include <common/inline_assembly.h>

sleeping_task_t * sleep_queue_head = 0;
sleeping_task_t * sleep_queue_tail = 0;

void add_to_sleep_queue(sleeping_task_t* sleeper){
  //breakpoint("add_to_sleep_queue");
  
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

  //breakpoint("ms_sleep_until");
  
  // Make sure wakeup time has not occurred already
  if (wake_time_ms < ms_since_boot()){
    return;
  }

  // Grab scheduler lock, postpone any task switches during this time
  lock_stuff();

  sleeping_task_t* new_sleeper = (sleeping_task_t*)kalloc(sizeof(sleeping_task_t), 0, kheap);
  if (!new_sleeper){
    printf("Failed to alloc memory for new sleeper!\n");
  }

  // Fill out sleeping task structure
  tcb_t* current_task = get_current_task();
  new_sleeper->task = current_task;
  new_sleeper->wake_time = wake_time_ms;

  // Append to queue
  add_to_sleep_queue(new_sleeper);

  // Unlock the scheduler
  unlock_stuff();

  // Block the task until it wakes
  block_curr_task("ms_sleep_until");
}

void ms_sleep(uint64_t sleep_duration){
  uint64_t current_time = ms_since_boot();
  uint64_t wake_time_ms = current_time + sleep_duration;
  printf("Current time: %d -- Sleep until: %d\n", (uint32_t)current_time, (uint32_t)wake_time_ms);
  ms_sleep_until(wake_time_ms);
}


void wake_sleeping_tasks(){

  // Get current time in millis
  uint64_t time_millis = ms_since_boot();

  // Lock scheduler , prevent task switches
  lock_stuff();

  // For all sleeping tasks, unblock them (no pre-empting), remove the node from the queue
  // Update sleep list, and de-allocate now-unused links
  while (sleep_queue_head && sleep_queue_head->wake_time <= time_millis){
    printf("wake_sleeping_task %d at %d\n", sleep_queue_head->task->task_id, time_millis);
    unblock_task(sleep_queue_head->task, 0);
    sleeping_task_t* to_delete = sleep_queue_head;
    sleep_queue_head = sleep_queue_head->next;
    kfree(to_delete, kheap);
  }

  // Unlock schedule
  unlock_stuff();
}
