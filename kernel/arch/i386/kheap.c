#include <kernel/kheap.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <kernel/boot_heap.h>
#include <common/inline_assembly.h>

heap_t* kheap = (heap_t*)0x0;

// ---------------------
// Helper Functions
// ---------------------

// Forward declarations
uint32_t GET_SIZE(void* ptr);

// Definitions
uint32_t IS_FREE(header_t* header){
  return !(header->size & 0x1);
}

header_t* GET_HDR(void* ptr) {
  return (header_t*)(((uint32_t)ptr) - sizeof(header_t));
}

footer_t* GET_FTR(void* ptr){
  return (footer_t*)(((uint32_t)ptr) + GET_SIZE(ptr));
}

void* GET_DATA(header_t* hdr){
  return (void*)((uint32_t)hdr + HDR_SIZE);
}

uint32_t GET_SIZE(void* ptr){
  return (GET_HDR(ptr)->size & (~0x1));
}

void* NEXT_BLOCK(void* ptr){
  // Add block size to the ptr, skip past the footer, skip past next block's header
  return (void*)((uint32_t)ptr + GET_SIZE(ptr) + FTR_SIZE + HDR_SIZE);
}

void* PREV_BLOCK(void* ptr){
  footer_t* prev_block_footer = (footer_t*)((uint32_t)ptr - (HDR_SIZE + FTR_SIZE));
  return (void*)((uint32_t)prev_block_footer->header + HDR_SIZE);
}

void SET_HDR(void* ptr, uint32_t size, uint32_t flag){
  header_t* header = GET_HDR(ptr);
  header->size = size | flag;
  header->magic = KHEAP_MAGIC;
}

void SET_FTR(void* ptr) {
  footer_t* footer = GET_FTR(ptr);
  footer->header = GET_HDR(ptr);
  footer->magic = KHEAP_MAGIC;
}

void SET_HDR_FTR(void* ptr, uint32_t size, uint32_t flag){
  SET_HDR(ptr, size, flag);
  SET_FTR(ptr);
}

uint32_t TOTAL_BLK_SIZE(uint32_t data_size){
  return (data_size + HDR_SIZE + FTR_SIZE);
}

uint32_t DATA_SIZE(uint32_t full_block_size){
  return (full_block_size - (HDR_SIZE + FTR_SIZE));
}

free_hdr_t* FREE_HDR_FROM_LIST(freelist_data_t* free_links){
  return (free_hdr_t*)((uint32_t)free_links - sizeof(header_t));
}

void LINK_UP_FREELIST(freelist_data_t* free_links){
  if (free_links->next){
    free_links->next->freelist_data.prev = FREE_HDR_FROM_LIST(free_links);
  }
  if (free_links->prev){
    free_links->prev->freelist_data.next = FREE_HDR_FROM_LIST(free_links);
  }
}

void REMOVE_FROM_FREELIST(freelist_data_t* free_links){
  if (free_links->next){
    free_links->next->freelist_data.prev = free_links->prev;
  }
  if (free_links->prev){
    free_links->prev->freelist_data.next = free_links->next;
  }
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

  heap_t* new_heap = (heap_t*)boot_alloc(sizeof(heap_t), 0);

  // breakpoint();

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
  // Set up initial heap as one giant free block
  uint32_t avail_space = DATA_SIZE(size);
  SET_HDR_FTR((void*)(start_addr + sizeof(header_t)), avail_space, FREE_FLAG);
  new_heap->freelist_head = (free_hdr_t*)start_addr;
  new_heap->freelist_head->freelist_data.next = 0x0;
  new_heap->freelist_head->freelist_data.prev = 0x0;

  return new_heap;
}

void* kalloc(uint32_t size, uint16_t align, heap_t* heap){

  free_hdr_t* free_itr = heap->freelist_head;
  while(free_itr && free_itr->header.size < size){
    free_itr = free_itr->freelist_data.next;
  }

  // If we reach the end of the list w/o finding a block
  if (!free_itr){
    return (void*)0x0;
  }

  // Get data ptr
  void* ptr = GET_DATA(&(free_itr->header));

  // The free block includes linked list info, which we need
  freelist_data_t freelist_links = free_itr->freelist_data;

  // Also save original block size
  uint32_t block_remainder = free_itr->header.size - size;

  // Setup the current block
  SET_HDR_FTR(ptr, size, USED_FLAG);

  breakpoint();

  // Split the block if there is enough room (16+ bytes usable space)
  // Otherwise, remove block from the free list
  if (block_remainder > TOTAL_BLK_SIZE(16)) {

    void* next_block = NEXT_BLOCK(ptr);
    SET_HDR_FTR(next_block, DATA_SIZE(block_remainder), FREE_FLAG);

    // Set up links
    freelist_data_t* new_links = (freelist_data_t*)next_block;
    *new_links = freelist_links;
    LINK_UP_FREELIST(new_links);
  } else {
    REMOVE_FROM_FREELIST(&freelist_links);
  }
  
  return free_itr;
  
}
