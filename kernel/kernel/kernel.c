#include <stdio.h>
#include <stdlib.h>
 
#include <kernel/tty.h>
#include <kernel/descriptor_table.h>
#include <kernel/idt.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <kernel/boot_heap.h>
#include <kernel/kheap.h>
 
void kernel_main(void) {

  // Setup GDT
  init_descriptor_tables();

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
  TEST_kheap();
}
