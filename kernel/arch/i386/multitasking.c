#include <kernel/multitasking.h>
#include <kernel/kheap.h>
#include <stdio.h>
#include <kernel/tss.h>
#include <string.h>
#include <kernel/paging.h>
#include <common/inline_assembly.h>

static uint32_t task_id_counter = 0;

tcb_t* curr_tcb = 0;
tcb_t* task_list_head = 0;
tcb_t* task_list_tail = 0;

void initialize_multitasking(){
  //breakpoint();
  curr_tcb = (tcb_t*)kalloc(sizeof(tcb_t), 0, kheap);
  if (!curr_tcb){
    printf("Err allocating initial tcb\n");
    return;
  }

  // Get fields for populating the initial task struct
  uint32_t esp;
  asm("mov %%esp, %0" : "=r"(esp));

  uint32_t cr3;
  asm("mov %%cr3, %0" : "=r"(cr3));

  curr_tcb->esp = esp;
  curr_tcb->esp0 = context_tss.esp0;
  curr_tcb->cr3 = cr3;
  curr_tcb->state = TASK_RUNNING;
  curr_tcb->task_id = task_id_counter++;
  curr_tcb->next_task = 0;
  curr_tcb->prev_task = 0;

  // Set first tcb to be the start of the task list
  task_list_head = curr_tcb;
  task_list_tail = curr_tcb;
}

tcb_t* create_kernel_task(void (*entry_EIP)()){
  //breakpoint("create_kernel_task");
  tcb_t* new_tcb = (tcb_t*)kalloc(sizeof(tcb_t), 0, kheap);
  if (!new_tcb){
    printf("Err allocating initial tcb\n");
    return 0;
  }

  //breakpoint();
  // Grab some memory for the process stack
  uint32_t stack_size = 1024;
  void* proc_stack = kalloc(stack_size, 0, kheap);
  //breakpoint();
  uint32_t stack_bottom = (uint32_t)proc_stack + stack_size; // start at the end

  // Space for registers we pop off the stack
  // Pop order: ebp, edi, esi, ebx, eip
  // For a new task, these registers should just be zero
  // eip should be the function pointer to the process entry point
  uint32_t initial_stack_size = (5 * 4);

  // Clean out the new stack to zero space for soon to be popped regs
  // We'll then set the eip separately
  // Note the pointer arithmetic: (proc_stack-5) = proc_stack - (5 * 4bytes)
  memset((uint32_t*)stack_bottom - 5, 0x0, initial_stack_size);

  //breakpoint();

  // Set the EIP for the new process
  (*((uint32_t*)stack_bottom - 1)) = (uint32_t)entry_EIP;

  // @TODO initialize this w/ function for new Page Directory
  page_directory_t* new_vaddr_space = (page_directory_t*)0x00106000;
  
  new_tcb->esp = stack_bottom - initial_stack_size;
  new_tcb->esp0 = stack_bottom;
  new_tcb->cr3 = (uint32_t)new_vaddr_space;
  new_tcb->state = TASK_READY;
  new_tcb->task_id = task_id_counter++;

  // When creating a new task, append to back of the list
  new_tcb->next_task = 0;
  new_tcb->prev_task = task_list_tail;
  task_list_tail->next_task = new_tcb;
  task_list_tail = new_tcb;

  //breakpoint("create_kernel_task end");

  return new_tcb;
}

tcb_t* get_current_task(){
  return curr_tcb;
}

uint32_t get_task_id(){
  return curr_tcb->task_id;
}

tcb_t* get_next_task(){
  tcb_t* next_task = curr_tcb->next_task;
  if (!next_task) {
    // Wrap around if at end of list
    return task_list_head;
  }

  return next_task;
}

void switch_to_next_task(){
  //breakpoint("switch_to_next_task");
  tcb_t* next_task = get_next_task();

  // If no other tasks, nothing to do
  if (!next_task || (next_task == curr_tcb)){
    return;
  }
  
  switch_to_task(next_task);
}

extern void switch_to_task_asm(tcb_t* new_task); // Assembly function
void switch_to_task(tcb_t* new_task){
  CLI();
  switch_to_task_asm(new_task);
  STI();
}
