#ifndef _BOOT_HEAP_H
#define _BOOT_HEAP_H

#include <stdint.h>

// ----------------------------------
// Module for Kernel Heap Operations
// ----------------------------------



// ----------------------------------
// Bootstrap Allocator
// Used before heap is set up
// ----------------------------------

void setup_boot_heap();
uint32_t boot_alloc(uint32_t size, uint32_t align);
uint32_t boot_alloc_frame();

#endif // _KHEAP_H
