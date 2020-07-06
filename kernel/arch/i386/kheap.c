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
  void* next_block = (void*)((uint32_t)ptr + GET_SIZE(ptr) + FTR_SIZE + HDR_SIZE);

  if ((uint32_t)next_block > kheap->heap_end){
    return 0x0;
  }

  return next_block;
}

void* PREV_BLOCK(void* ptr){
  footer_t* prev_block_footer = (footer_t*)((uint32_t)ptr - (HDR_SIZE + FTR_SIZE));

  // If we try to go back past the beginning of the heap
  if ((uint32_t)prev_block_footer < kheap->heap_start){
    return 0x0;
  }
  
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

void MARK_FREE(void* ptr){
  header_t* hdr = GET_HDR(ptr);
  hdr->size &= (~0x1);
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

    // Get next block, make sure it's valid
    void* next_block = NEXT_BLOCK(ptr);
    if (!next_block){
      return 0x0;
    }

    // Setup the next block
    SET_HDR_FTR(next_block, DATA_SIZE(block_remainder), FREE_FLAG);

    // Set up links
    freelist_data_t* new_links = (freelist_data_t*)next_block;
    *new_links = freelist_links;
    LINK_UP_FREELIST(new_links);
  } else {
    REMOVE_FROM_FREELIST(&freelist_links);
  }
  
  return ptr;
  
}

uint8_t coalesce(void* ptr){

  uint8_t performed_coalesce = 0;

  // If prev block is free, coalesce left
  // Nothing to do for freelist links; they're already set up
  
  header_t* prev_ptr = PREV_BLOCK(ptr);
  header_t* prev_hdr = (prev_ptr) ? GET_HDR(prev_ptr) : 0x0;
  if (prev_hdr && IS_FREE(prev_hdr)){
    // Add the size of the newly freed block
    // Include size of now unused new block's header, prev block's footer
    prev_hdr->size += TOTAL_BLK_SIZE(GET_SIZE(ptr));

    // ptr's footer now belongs to prev block
    GET_FTR(ptr)->header = prev_hdr;

    // Set ptr to ptr of coalesced block, for use in coalesce-right
    ptr = GET_DATA(prev_hdr);

    // Indicate that we performed some coalesce
    performed_coalesce++;
  }

  // If next block is free, coalesce right
  // If we're at the end of the heap, don't coalesce right --
  // return result of coalesce left
  void* next_ptr = NEXT_BLOCK(ptr);
  if (!next_ptr){
    return performed_coalesce;
  }
  header_t* next_hdr = GET_HDR(next_ptr);
  if (IS_FREE(next_hdr)){
    // Add size of right block to our current block size
    header_t* cur_hdr = GET_HDR(ptr);
    cur_hdr->size += TOTAL_BLK_SIZE(GET_SIZE(next_ptr));

    // Set right block's footer to our header
    SET_FTR(ptr);

    // Fix up freelist links
    freelist_data_t* next_ptr_links = (freelist_data_t*)next_ptr;
    freelist_data_t* cur_ptr_links = (freelist_data_t*)ptr;
    if (next_ptr_links && next_ptr_links->next){
      if (cur_ptr_links && cur_ptr_links->next){
	// If cur ptr is already on freelist, remove next ptr
	REMOVE_FROM_FREELIST(next_ptr_links);
      } else {
	// If ptr has no free links, steal them from next ptr
	*cur_ptr_links = *next_ptr_links;
	LINK_UP_FREELIST(cur_ptr_links);
      }
    }

    // Indicate that we performed some coalesce
    performed_coalesce++;
  }

  return performed_coalesce;
}

void kfree(void* ptr, heap_t* heap){

  breakpoint();

  // Make sure ptr is non-null and allocated
  if (!ptr || IS_FREE(GET_HDR(ptr))){
    return;
  }

  // Mark the individual block as free
  MARK_FREE(ptr);

  uint8_t performed_coalesce = coalesce(ptr);

  // If we didn't coalesce the block, we need to manually add to freelist
  // Just append to the beginning of the list
  if (!performed_coalesce){
    freelist_data_t* ptr_links = (freelist_data_t*)ptr;
    ptr_links->prev = 0x0;
    ptr_links->next = heap->freelist_head;
    heap->freelist_head->freelist_data.prev = (free_hdr_t*)(GET_HDR(ptr));
    heap->freelist_head = (free_hdr_t*)(GET_HDR(ptr));
  }

}
