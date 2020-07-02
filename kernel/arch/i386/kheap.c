#include <kernel/kheap.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <kernel/boot_heap.h>
#include <common/inline_assembly.h>

heap_t* kheap = (heap_t*)0x0;

// --------------
// Helper Functions
// -----------------

header_t* GET_HDR(void* ptr) {
  return (header_t*)(((uint32_t)ptr) - sizeof(header_t));
}

footer_t* GET_FTR(void* ptr){
  header_t* hdr = GET_HDR(ptr);
  return (footer_t*)(((uint32_t)ptr) + hdr->size);
}

void* NEXT_BLOCK(void* ptr){
  return (void*)((uint32_t)ptr + GET_HDR(ptr)->size + FTR_SIZE + HDR_SIZE);
}

// -----------------------
// Main Functionality
// -----------------------


void setup_kheap(){

  // Need boot heap to place heap structures
  setup_boot_heap();
  kheap = create_heap(0xD0000000, KHEAP_INITIAL_SIZE, 0x3);

}

heap_t* create_heap(uint32_t start_addr, uint32_t size, uint8_t flags){
    breakpoint();

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

  // Pointer is always to the block of mem itself, not the header
  // Use macro to get the header position when using this
  new_heap->freelist_ptr = (void*)(start_addr + sizeof(header_t));

  // Set up initial heap as one giant free block
  header_t* chunk_header = GET_HDR(new_heap->freelist_ptr);
  chunk_header->size = size - (HDR_SIZE + FTR_SIZE);
  chunk_header->magic = KHEAP_MAGIC;
  footer_t* chunk_footer = GET_FTR(new_heap->freelist_ptr);
  chunk_footer->header = chunk_header;
  chunk_footer->magic = KHEAP_MAGIC;
  
  return new_heap;
}

void* kalloc(uint32_t size, uint16_t align, heap_t* heap){

  void* free_itr = heap->freelist_ptr;
  while(GET_HDR(free_itr)->size & 0x1){
    free_itr = NEXT_BLOCK(free_itr);

    if ((uint32_t)free_itr >= heap->heap_end){
      return (void*)0x0;
    }
  }

  return free_itr;
  
}
