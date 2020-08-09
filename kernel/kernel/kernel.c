#include <stdio.h>
#include <stdlib.h>
 
#include <kernel/tty.h>
#include <kernel/descriptor_table.h>
#include <kernel/idt.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <kernel/boot_heap.h>
#include <kernel/kheap.h>
#include <kernel/ps2controller.h>
#include <kernel/keyboard.h>
#include <kernel/PIT_Timer.h>
#include <kernel/multitasking.h>
#include <common/inline_assembly.h>

extern void jump_usermode();

void do_something_else(){
  printf("doing something else!\n");
}

void test_mt(){
  printf("Made it to new task!\n");

  STI();

  int bar = 0;
  while(1){
    // infinite loop
    key_input_t key;
    if (pop_key_event(&key) && !key.rel_if_set){
      printf("Bar(%d): %d - %c\n", get_task_id(), bar++, key.ascii_value);
      switch_to_next_task();
    }
  }
    
}
 
void kernel_main(void) {

  // Setup GDT and TSS
  init_descriptor_tables();
  //  breakpoint();

  // Setup IDT
  idt_init();

  // Setup screen/graphics, print
  terminal_initialize();
  printf("Hello, kernel World!\n");

  // Setup physical memory manager
  setup_pmm();

  // Set up kernel heap
  setup_kheap();

  // Run tests
  //TEST_kheap();

  // Initialize hardware
  initialize_PIT_timer(100);
  initialize_ps2_controller();
  initialize_keyboard_state();

  // Multitasking
  initialize_multitasking();
  create_kernel_task(&test_mt);
  create_kernel_task(&test_mt);
  
  switch_to_next_task();

  printf("Returned!\n");

  // --------------------------------------------
  // Won't go past this point if we switch task
  // -------------------------------------------

  int foo = 0;
  while(1){
    // infinite loop
    key_input_t key;
    if (pop_key_event(&key) && !key.rel_if_set){
      printf("Foo(%d): %d - %c\n", get_task_id(), foo++, key.ascii_value);
      switch_to_next_task();
    }
  }
}
