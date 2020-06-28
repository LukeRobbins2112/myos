#include "kernel/kheap.h"

// ------------------
// _kernel_end defined in linker
extern uint32_t _kernel_end;
static uint32_t next_alloc = (void*)&_kernel_end;


uint32_t boot_alloc(uint32_t size, uint32_t align){

  if (align && (next_alloc & 0x00000FFF)){
    next_alloc += 0xFFF;
    next_alloc &= 0xFFFFF000;
  }

  uint32_t new_addr = next_alloc;
  next_alloc += size;
  return new_addr;
}

uint32_t boot_alloc_frame(){
  return boot_alloc(0x1000, 1);
}

