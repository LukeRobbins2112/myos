#include <kernel/kheap.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <kernel/boot_heap.h>

heap_t* kheap = (heap_t*)0x0;

void setup_kheap(){

  // Need boot heap to place heap structures
  setup_boot_heap();
  kheap = create_heap(0xD0000000, KHEAP_INITIAL_SIZE, 0x3);

}

heap_t* create_heap(uint32_t start_addr, uint32_t size, uint8_t flags){

  heap_t* new_heap = (heap_t*)boot_alloc(sizeof(heap_t), 0);
  
  // Make sure start address is page aligned
  if (start_addr & 0x00000FFF){
    start_addr += 0x00000FFF;
    start_addr &= 0xFFFFF000;
  }

  new_heap->heap_start = start_addr;
  new_heap->prog_break = start_addr;
  new_heap->heap_end = start_addr + size;
  new_heap->max_size = 0x1000000;
  new_heap->flags = flags;

  
  return new_heap;
}

void* kalloc(uint32_t size, uint16_t align, heap_t* heap){
  void* res = (void*)heap->prog_break;
  heap->prog_break += size;
  return res;
}
