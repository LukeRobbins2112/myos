#ifndef _TASK_CONTROL_BLOCK_H
#define _TASK_CONTROL_BLOCK_H

#include <stdint.h>

typedef enum task_state {
 TASK_STOPPED    = 0,
 TASK_RUNNING    = 1,
 TASK_READY      = 2,
 TASK_BLOCKED    = 3,
 TASK_TERMINATED = 4
} TASK_STATE;

typedef struct TCB {

  uint32_t esp;   // 
  uint32_t esp0;
  uint32_t cr3;
  TASK_STATE state;
  uint32_t task_id;

  // Linked List pointers
  struct TCB* prev_task;
  struct TCB* next_task;
} tcb_t;



#endif // _TASK_CONTROL_BLOCK_H
