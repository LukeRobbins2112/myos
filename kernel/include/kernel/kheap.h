#ifndef _KHEAP_H
#define _KHEAP_H

#include <stdint.h>

// ----------------------------------
// Module for Kernel Heap Operations
// ----------------------------------



// ----------------------------------
// Bootstrap Allocator
// Used before heap is set up
// ----------------------------------

uint32_t boot_alloc(uint32_t size, uint32_t align);
uint32_t boot_alloc_frame();

#endif // _KHEAP_H
