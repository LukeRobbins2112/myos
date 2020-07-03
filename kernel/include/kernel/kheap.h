// **************************************************************
// **************************************************************
// Module for managing kernel heap, dynamic memory allocation
// **************************************************************
// **************************************************************

#ifndef _KHEAP_H
#define _KHEAP_H


#include <stdint.h>

// --------------------------------------------------------------
// Constant Definitions
// --------------------------------------------------------------

#define KHEAP_START          0xD0000000
#define KHEAP_INITIAL_SIZE   0x1000
#define KHEAP_MAGIC          0xFACEB00C

// -------------------------------------------------------------
// Data structure definitions
// -------------------------------------------------------------

typedef struct header {
  // Size = block size - sizeof(header_t) - sizeof(footer_t)
  uint32_t size;    // Low bit is alloc flag. 0 = free, 1 = allocated
  uint32_t magic;
} header_t;

typedef struct freelist_data {
  free_hdr_t* next;
  free_hdr_t* prev;
} freelist_data_t;

typedef struct free_hdr {
  header_t header;
  freelist_data_t freelist_data;
} free_hdr_t;

typedef struct footer {
  header_t* header;
  uint32_t magic;
} footer_t;

typedef struct heap {
  uint32_t heap_start;              // Start of the heap
  uint32_t prog_break;              // Current end of alloc'd memory
  uint32_t heap_end;                // End of available heap space; can be expanded
  uint32_t max_size;                // Heap cannot grow larger than this
  free_hdr_t* freelist_head;   // For iterating through available blocks
  uint8_t flags;                    // 0-bit = kernel, 1-bit = read/write
} heap_t;

// ------------------------------------------------------------
// Heap Function Declarations
// ------------------------------------------------------------

void setup_kheap();
heap_t* create_heap(uint32_t start_addr, uint32_t size, uint8_t flags);
void* kalloc(uint32_t size, uint16_t align, heap_t* heap);
void kfree(void* ptr, heap_t* heap);

// --------------------------------------------------------------
// Helper Macros
// --------------------------------------------------------------
#define HDR_SIZE (sizeof(header_t))
#define FTR_SIZE (sizeof(footer_t))
#define FREE_FLAG 0x0
#define USED_FLAG 0x1


// --------------------------------------------------------------
// External variables
// --------------------------------------------------------------

extern heap_t* kheap;


#endif // _KHEAP_H
