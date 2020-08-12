#ifndef _MULTITASKING_H
#define _MULTITASKING_H

#include <kernel/task_control_block.h>

// -----------------
// Data
// -----------------

extern tcb_t* curr_tcb;
extern tcb_t* task_list_head;
extern tcb_t* task_list_tail;

extern tcb_t* blocked_tasks;

// ----------------------------------
// Main Multitasking API
// ----------------------------------

void initialize_multitasking();
tcb_t* create_kernel_task(void (*entry_EIP)());
void switch_to_task(tcb_t* new_task);
void switch_to_next_task();
void schedule();
void block_curr_task();
void unblock_task(tcb_t* task, uint8_t preempt);


// ----------------------------------
// Other Global Functions
// ----------------------------------
uint32_t get_task_id();
tcb_t* get_current_task();
void lock_scheduler();
void unlock_scheduler();
void lock_stuff();
void unlock_stuff();
#endif // _MULTITASKING_H
