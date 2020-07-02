// **************************************************************
// **************************************************************
// Module for managing kernel heap, dynamic memory allocation
// **************************************************************
// **************************************************************

#ifndef _KHEAP_H
#define _KHEAP_H


#include <stdint.h>

// --------------------------------------------------------------
// Macros and Constant Definitions
// --------------------------------------------------------------

#define KHEAP_START          0xD0000000
#define KHEAP_INITIAL_SIZE   0x1000
#define KHEAP_MAGIC          0xFACEB00C


// -------------------------------------------------------------
// Data structure definitions
// -------------------------------------------------------------

typedef struct header {
  uint32_t size;
  uint32_t magic;
} header_t;

typedef struct footer {
  header_t* header;
  uint32_t magic;
} footer_t;

typedef struct heap {
  uint32_t heap_start; // Start of the heap
  uint32_t prog_break; // Current end of alloc'd memory
  uint32_t heap_end;   // End of available heap space; can be expanded
  uint32_t max_size;   // Heap cannot grow larger than this
  uint8_t flags;       // 0-bit = kernel, 1-bit = read/write
} heap_t;

// ------------------------------------------------------------
// Heap Function Declarations
// ------------------------------------------------------------

void setup_kheap();
heap_t* create_heap(uint32_t start_addr, uint32_t size, uint8_t flags);
void* kalloc(uint32_t size, uint16_t align, heap_t* heap);
void kfree(void* ptr, heap_t* heap);

// --------------------------------------------------------------
// External variables
// --------------------------------------------------------------

extern heap_t* kheap;


#endif // _KHEAP_H
