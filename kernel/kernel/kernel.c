#include <stdio.h>
 
#include <kernel/tty.h>
#include <kernel/descriptor_table.h>
#include <kernel/idt.h>
#include <kernel/pmm.h>
 
void kernel_main(void) {

  // Setup GDT
  init_descriptor_tables();

  // Setup IDT
  idt_init();

  // Setup physical memory manager
  setup_pmm();

  // Test page fault handler
  uint32_t *ptr = (uint32_t *)0xA0000000;
  uint32_t do_page_fault = *ptr;
  
  // Setup screen/graphics, print
  terminal_initialize();
  printf("Hello, kernel World!\n");
  printf("Initialized descriptor tables\n");

  if (do_page_fault){
   printf("bar is valid\n");
  }
}
