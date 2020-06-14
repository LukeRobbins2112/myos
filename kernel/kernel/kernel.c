#include <stdio.h>
 
#include <kernel/tty.h>
#include <kernel/descriptor_table.h>
#include <kernel/idt.h>
 
void kernel_main(void) {

  // Setup GDT
  init_descriptor_tables();

  // Setup IDT
  idt_init();

  // Setup screen/graphics, print
  terminal_initialize();
  printf("Hello, kernel World!\n");
  printf("Initialized descriptor tables\n");
}
