#ifndef _MULTITASKING_H
#define _MULTITASKING_H

#include <kernel/task_control_block.h>

// -----------------
// Data
// -----------------

extern tcb_t* curr_tcb;
extern tcb_t* task_list_head;
extern tcb_t* task_list_tail;



// ----------------------------------
// Main Multitasking API
// ----------------------------------

void initialize_multitasking();
tcb_t* create_kernel_task(void (*entry_EIP)());
void switch_to_task(tcb_t* new_task);
void switch_to_next_task();

// ----------------------------------
// Other Global Functions
// ----------------------------------
uint32_t get_task_id();
void unlock_scheduler();

#endif // _MULTITASKING_H
