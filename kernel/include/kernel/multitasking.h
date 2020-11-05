#ifndef _MULTITASKING_H
#define _MULTITASKING_H

#include <kernel/task_control_block.h>

// -----------------------------------------
// Constants
// -----------------------------------------

#define TIME_SLICE_LENGTH_MS 100000 // 5 seconds (!)

// -----------------
// Data
// -----------------

extern tcb_t* curr_tcb;        // "Running" TCB
extern tcb_t* task_list_head; 
extern tcb_t* task_list_tail;

extern tcb_t* blocked_tasks;
extern tcb_t* terminated_tasks;

// ----------------------------------
// Main Multitasking API
// ----------------------------------

void initialize_multitasking();
tcb_t* create_kernel_task(void (*entry_EIP)());
void switch_to_task(tcb_t* new_task);
void switch_to_next_task();
void schedule();
void schedule_under_lock();
void block_curr_task();
void unblock_task(tcb_t* task, uint8_t preempt);
void terminate_task();

// ----------------------------------
// Other Global Functions
// ----------------------------------
uint32_t get_task_id();
tcb_t* get_current_task();
void lock_scheduler();
void unlock_scheduler();
void lock_stuff();
void unlock_stuff();
void dump_lock_info();
void create_cleanup_task();

#endif // _MULTITASKING_H
