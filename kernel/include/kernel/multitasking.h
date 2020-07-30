#ifndef _MULTITASKING_H
#define _MULTITASKING_H

#include <kernel/task_control_block.h>

// -----------------
// Data
// -----------------

extern tcb_t* curr_tcb;

// -----------------
// 
// -----------------

void initialize_multitasking();
tcb_t* create_kernel_task(void (*entry_EIP)());


#endif // _MULTITASKING_H
