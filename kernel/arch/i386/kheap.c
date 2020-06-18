#include "kernel/kheap.h"

// Used for placement new
// end is defined in the linker script.
extern uint32_t end;
uint32_t placement_address = (uint32_t)&end;

//--------------------------------------------------
// Memory Allocation Functions
//--------------------------------------------------

uint32_t kmalloc_int(uint32_t sz, int align, uint32_t* phys){

  // If not 4k-aligned, set to the next available 4k-aligned address
  if (align == 1 && (placement_address & 0x00000FFF)){
    placement_address &= 0xFFFFF000;
    placement_address += 0x1000;
  }

  // Set physical
  if (phys){
    *phys = placement_address;
  }

  // Return current placement_address as pointer
  // Update placement_address to point to past this chunk
  uint32_t newChunk = placement_address;
  placement_address += sz;
  return newChunk;
}


// Wrapper functions

uint32_t kmalloc(uint32_t sz){
  return kmalloc_int(sz, 0, 0);
}

uint32_t kmalloc_a(uint32_t sz){
  return kmalloc_int(sz, 1, 0);
}

uint32_t kmalloc_p(uint32_t sz, uint32_t* phys){
  return kmalloc_int(sz, 0, phys);
}

uint32_t kmalloc_ap(uint32_t sz, uint32_t* phys){
  return kmalloc_int(sz, 1, phys);
}
