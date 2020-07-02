#include <stdio.h>
 
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

  // Set up virtual memory manager
  setup_kheap();

  // Set up heap
  // Test page fault handler on kalloc
  uint32_t *ptr = (uint32_t *)kalloc(32, 0, kheap);
  uint32_t do_page_fault = *ptr;
  do_page_fault++;
  
  // Setup screen/graphics, print
  terminal_initialize();
  printf("Hello, kernel World!\n");
  printf("Initialized descriptor tables\n");

  if (do_page_fault){
   printf("bar is valid\n");
  }
}
