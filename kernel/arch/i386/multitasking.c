#include <kernel/multitasking.h>
#include <kernel/kheap.h>
#include <stdio.h>
#include <kernel/tss.h>
#include <string.h>
#include <kernel/paging.h>
#include <common/inline_assembly.h>
#include <kernel/vmm.h>
#include <kernel/timer.h>

// ----------------------------------------
// Lock Data
// ------------------------------
static uint32_t IRQ_disable_counter = 0;
uint32_t postpone_task_switches_counter = 0;
uint8_t task_switches_postponed_flag = 0;
// -----------------------------------------

// ------------------------------
// Manage Running & Ready Tasks
// ------------------------------
tcb_t* curr_tcb = 0;
tcb_t* task_list_head = 0;
tcb_t* task_list_tail = 0;
// ------------------------------

// ------------------------------
// Manage Blocked & Sleeping Tasks
// ------------------------------
tcb_t* blocked_tasks = 0;
// ------------------------------

// ------------------------------
// Misc. Data
// ------------------------------
static uint32_t task_id_counter = 0;
// ------------------------------

// Declarations
void append_ready_task(tcb_t* ready_task);

void lock_scheduler(){
#ifndef SMP
  CLI();
  IRQ_disable_counter++;
  //printf("lock_scheduler(): IRQ_Disable_counter = %d\n", IRQ_disable_counter);
#endif // #ifndef SMP
}

void unlock_scheduler(){
#ifndef SMP
  IRQ_disable_counter--;
  if (IRQ_disable_counter == 0){
    STI();
  }
  //printf("unlock_scheduler(): IRQ_Disable_counter = %d\n", IRQ_disable_counter);
#endif // #ifndef SMP
}



void lock_stuff(){
#ifndef SMP
  CLI();
  IRQ_disable_counter++;
  postpone_task_switches_counter++;
  //printf("lock_stuff(): IRQ_Dis_ctr = %d -- Postpone = %d\n", IRQ_disable_counter, postpone_task_switches_counter);
#endif // #ifndef SMP
}

void unlock_stuff(){
#ifndef SMP
  postpone_task_switches_counter--;

  // If there are no longer any locks...
  if (postpone_task_switches_counter == 0){
    // ...and a task switch has been delayed
    if (task_switches_postponed_flag != 0){
      task_switches_postponed_flag = 0;
      //breakpoint("unlock_stuff() switching");
      printf("unlock_stuff(): IRQ_Dis_ctr = %d -- Postpone = %d -- Flag = %d\n", IRQ_disable_counter, postpone_task_switches_counter, task_switches_postponed_flag);
      //unlock_scheduler();
      schedule();
    }
  }
  
  IRQ_disable_counter--;
  if (IRQ_disable_counter == 0){
    STI();
  }
  //printf("unlock_stuff(): IRQ_Dis_ctr = %d -- Postpone = %d -- Flag = %d\n", IRQ_disable_counter, postpone_task_switches_counter, task_switches_postponed_flag);
#endif // #ifndef SMP
}


void dump_lock_info(){
printf("Lock Info: IRQ_Dis_ctr = %d -- Postpone = %d -- Flag = %d\n", IRQ_disable_counter, postpone_task_switches_counter, task_switches_postponed_flag);
}

// ---------------------------------
// Main Multiaskings & Scheduling API
// ---------------------------------

void initialize_multitasking(){
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

  // First tcb is running, no need to add to ready queue
}

tcb_t* create_kernel_task(void (*entry_EIP)()){
  tcb_t* new_tcb = (tcb_t*)kalloc(sizeof(tcb_t), 0, kheap);
  if (!new_tcb){
    printf("Err allocating initial tcb\n");
    return 0;
  }

  // Grab some memory for the process stack
  uint32_t stack_size = 1024;
  void* proc_stack = kalloc(stack_size, 0, kheap);
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

  // Set the EIP for the new process
  (*((uint32_t*)stack_bottom - 1)) = (uint32_t)entry_EIP;

  // @TODO initialize this w/ function for new Page Directory
  page_directory_t* new_vaddr_space = (page_directory_t*)get_physaddr((uint32_t)get_page_directory());
  
  new_tcb->esp = stack_bottom - initial_stack_size;
  new_tcb->esp0 = stack_bottom;
  new_tcb->cr3 = (uint32_t)new_vaddr_space;
  new_tcb->state = TASK_READY;
  new_tcb->task_id = task_id_counter++;

  // Add to the ready queue
  append_ready_task(new_tcb);

  return new_tcb;
}

tcb_t* get_current_task(){
  return curr_tcb;
}

uint32_t get_task_id(){
  return curr_tcb->task_id;
}

void append_ready_task(tcb_t* ready_task){
  if (!task_list_head){
    task_list_head = ready_task;
    task_list_tail = ready_task;
    ready_task->next_task = 0;
    ready_task->prev_task = 0;
    return;
  }

  // Set up pointers to add to linked list
  ready_task->next_task = 0;
  ready_task->prev_task = task_list_tail;
  task_list_tail->next_task = ready_task;

  // ready_task is now at the end of the list
  task_list_tail = ready_task;
}

tcb_t* get_next_task(){
  tcb_t* next_task = task_list_head;
  return next_task;
}

void block_curr_task(){
  lock_scheduler();
  blocked_tasks = curr_tcb;
  switch_to_next_task();
  unlock_scheduler();
}

void unblock_task(tcb_t* task, uint8_t preempt){
  lock_scheduler();

  // If we're not postponing, and if either:
  // task_list is not null or we explicitly ask to preempt
  if ((postpone_task_switches_counter == 0) && (!task_list_head || preempt)){
    // Only one task was running
    
    // Add current task to ready list
    append_ready_task(curr_tcb);
    curr_tcb->state = TASK_READY;

    // Pre-empty with newly unblocked task
    task->state = TASK_RUNNING;
    switch_to_task(task);
  } else {
    // At least one task on ready queue, don't pre-empt
    append_ready_task(task);
  }
  unlock_scheduler();
}

void schedule(){
  //breakpoint("schedule");
  // Add current task to ready list
  append_ready_task(curr_tcb);
  curr_tcb->state = TASK_READY;

  // Pop new task off of ready list, switch to it
  switch_to_next_task();
}

void schedule_under_lock(){
  lock_scheduler();
  schedule();
  unlock_scheduler();
}

void switch_to_next_task(){
  // If there are still outstanding locks, don't switch yet
  if (postpone_task_switches_counter != 0){
    task_switches_postponed_flag = 1;
    return; 
  }
  
  tcb_t* next_task = get_next_task();

  // If there is another task, switch to it
  if (next_task){
    //breakpoint("there is a next task");
    // Update ready list head
    task_list_head = next_task->next_task;
    
    // Switch to new task
    next_task->state = TASK_RUNNING;
    switch_to_task(next_task);
  } else if (curr_tcb->state == TASK_RUNNING) {
    // Keep running if no other tasks
    printf("No more tasks\n");
  } else {
    printf("No more tasks\n");
    // Current running task blocked, no other tasks --> idle
    tcb_t* task = curr_tcb;
    curr_tcb = 0; // NULL
    //uint64_t idle_start_time = ms_since_boot(); // For power management

    // Do nothing while waiting for a task to unblock & become ready
    // This won't happen until we get an IRQ (e.g. sleeping task wakes)
    // IRQs are disabled, but we have to enable them to break this loop
    // Leave postponed_flag set b/c we want to stay here until we're done

    breakpoint("NO MORE TASKS");
    do {
      STI();
      HLT();
      CLI();
    } while (task_list_head == 0);

    // Give the task back
    curr_tcb = task;

    // Switch to the newly-unblocked task
    // Unless that task was the one we borrowed here...
    // .. in that case we're already on it; just return
    
    task = task_list_head;
    task_list_head = task->next_task;
    if (task != curr_tcb){
      task->state = TASK_RUNNING;
      switch_to_task(task);
    }
  }


}

extern void switch_to_task_asm(tcb_t* new_task); // Assembly function
void switch_to_task(tcb_t* new_task){
  if (postpone_task_switches_counter != 0) {
    task_switches_postponed_flag = 1;
    return;
  }

  switch_to_task_asm(new_task);
}
