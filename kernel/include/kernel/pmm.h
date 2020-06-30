#ifndef _PMM_H
#define _PMM_H

#include <stdint.h>
#include "kernel/paging.h"
#include "kernel/kheap.h"

// ** IMPORTANT ** Physical memory size
#define PHYSICAL_MEM_SIZE 0x8000000 // 128MB
#define FRAME_SIZE 0x1000

// ----------------------------------------
// Data
// ----------------------------------------

extern uint32_t num_frames;
extern uint32_t* frames;

// ----------------------------------------
// Bitset Helper Macros
// ----------------------------------------

#define FRAME_BITSET_FROM_ADDR(a)  ((a)/(8*4))
#define FRAME_BIT_OFFSET_FROM_ADDR(a) ((a)%(8*4))

// ----------------------------
// Function Declarations
// ----------------------------
void alloc_table(page_table_t* page_table, int is_kernel, int is_writeable);
void alloc_frame(page_t* page, int is_kernel, int is_writeable);
void free_frame(page_t* page);
uint32_t first_frame();
void setup_pmm();

#endif // _PMM_H
