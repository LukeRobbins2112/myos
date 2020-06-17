#ifndef _K_HEAP_H
#define _K_HEAP_H

#include <stdint.h>


uint32_t kmalloc_int(uint32_t sz, int align, uint32_t* phys);

// Some wrapper functions for kmalloc
uint32_t kmalloc(uint32_t sz);                      // Basic malloc
uint32_t kmalloc_a(uint32_t sz);                    // Aligned 4k
uint32_t kmalloc_p(uint32_t sz, uint32_t* phys);    // Physical
uint32_t kmalloc_ap(uint32_t sz, uint32_t* phys);   // Aligned physical




#endif // _K_HEAP_H
