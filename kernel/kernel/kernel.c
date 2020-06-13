#include <stdio.h>
 
#include <kernel/tty.h>
#include <kernel/descriptor_table.h>
 
void kernel_main(void) {

  // Setup GDT
  init_descriptor_tables();

  // Setup screen/graphics, print
  terminal_initialize();
  printf("Hello, kernel World!\n");
  printf("Initialized descriptor tables\n");
}
