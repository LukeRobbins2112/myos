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
 
void kernel_main(void) {

  // Setup GDT
  init_descriptor_tables();
  breakpoint();

  // Setup IDT
  idt_init();

  // Setup physical memory manager
  setup_pmm();

  // Set up kernel heap
  setup_kheap();

  // Setup screen/graphics, print
  terminal_initialize();
  printf("Hello, kernel World!\n");

  // Run tests
  //TEST_kheap();

  initialize_PIT_timer(100);
    
  initialize_ps2_controller();
  initialize_keyboard_state();
  while(1){
    // infinite loop
    key_input_t key;
    if (pop_key_event(&key) && !key.rel_if_set){
      printf("%c", key.ascii_value);
    }
  }
}
