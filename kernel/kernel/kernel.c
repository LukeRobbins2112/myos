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
#include <kernel/timer.h>
#include <kernel/multitasking.h>
#include <common/inline_assembly.h>
#include <kernel/sleep.h>
#include <kernel/ata.h>

extern void jump_usermode();

void do_something_else(){
  printf("doing something else!\n");
}

void test_mt(){
  //printf("Made it to new task %d!\n", get_task_id());

  int bar = 0;
  while(1){
    // infinite loop
    key_input_t key;
    if (pop_key_event(&key) && !key.rel_if_set){
      if (key.ascii_value == 'u'){
	//schedule();
      }
      if (key.ascii_value == 's'){
	ms_sleep(100000);
      }
      if (key.ascii_value == 'w'){
	terminate_task();
      }
      if (key.ascii_value == 'p'){
	printf("Bar(%d): %d - %c\n", get_task_id(), bar++, key.ascii_value);
      }
      if (key.ascii_value == 'l'){
	print_last_line();
      }
      if (key.special_key && key.key_code == UP_ARROW){
	scroll_up();
      }
      if (key.special_key && key.key_code == DOWN_ARROW){
	scroll_down();
      }
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
  initialize_PIT_timer(PIT_OUTPUT_FREQ);
  initialize_ps2_controller();
  initialize_keyboard_state();

  // Initialize disk
  detect_and_init();

  read_pio(1, 10, 0);

  // Multitasking
  initialize_multitasking();
  create_cleanup_task();
  for (int i = 0; i < 2; i++){
    create_kernel_task(&test_mt); // TID = 8
  }
  

  
  //schedule();
  //block_curr_task();
  //ms_sleep(100000);
  //schedule_under_lock();
  

  printf("Returned!\n");

  // --------------------------------------------
  // Won't go past this point if we switch task
  // -------------------------------------------

  int foo = 0;
  while(1){
    // infinite loop
    key_input_t key;
    if (pop_key_event(&key) && !key.rel_if_set){
      if (key.ascii_value == 'u'){
	//schedule();
      }
      if (key.ascii_value == 'w'){
	//wake_sleeping_tasks();
      }
      if (key.ascii_value == 'w'){
	//terminate_task();
      }
      if (key.ascii_value == 'p'){
	printf("Foo(%d): %d - %c\n", get_task_id(), foo++, key.ascii_value);
      }
      if (key.ascii_value == 'l'){
	print_last_line();
      }
      if (key.special_key && key.key_code == UP_ARROW){
	scroll_up();
      }
      if (key.special_key && key.key_code == DOWN_ARROW){
	scroll_down();
      }
    }
  }
}
