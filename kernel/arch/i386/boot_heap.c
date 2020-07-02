#include <kernel/boot_heap.h>
#include <common/inline_assembly.h>

// ------------------
// _kernel_end defined in linker
extern uint32_t k_workspace_end;
static uint32_t next_alloc = (uint32_t)0x0;

void setup_boot_heap(){
  breakpoint();
  next_alloc = k_workspace_end;
}


uint32_t boot_alloc(uint32_t size, uint32_t align){

  if (align && (next_alloc & 0x00000FFF)){
    next_alloc += 0x00000FFF;
    next_alloc &= 0xFFFFF000;
  }

  uint32_t new_addr = next_alloc;
  next_alloc += size;
  return new_addr;
}

uint32_t boot_alloc_frame(){
  return boot_alloc(0x1000, 1);
}

