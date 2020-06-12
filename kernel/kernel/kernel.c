#include <stdio.h>
 
#include <kernel/tty.h>
#include <kernel/descriptor_table.h>
 
void kernel_main(void) {
	terminal_initialize();
	printf("Hello, kernel World!\n");
	init_descriptor_tables();
	printf("Initialized descriptor tables\n");
}
