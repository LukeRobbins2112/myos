#include <kernel/kheap.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <kernel/boot_heap.h>
#include <common/inline_assembly.h>
#include <common/testing.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
    printf("WARNING: NEXT_BLOCK %x is beyond heap end\n", (uint32_t)next_block);
  }

  return next_block;
}

void* PREV_BLOCK(void* ptr){
  footer_t* prev_block_footer = (footer_t*)((uint32_t)ptr - (HDR_SIZE + FTR_SIZE));

  // If we try to go back past the beginning of the heap
  if ((uint32_t)prev_block_footer < kheap->heap_start){
    printf("ERROR: PREV_BLOCK is before heap start\n");
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

void ZERO_FREELIST_LINKS(void* ptr){
  freelist_data_t* free_links = (freelist_data_t*)ptr;
  free_links->next = 0x0;
  free_links->prev = 0x0;
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

  // If this was the kheap->freelist_headr, update that to next
  if ((FREE_HDR_FROM_LIST(free_links) == kheap->freelist_head)){
    kheap->freelist_head = free_links->next;
  }
}

uint32_t GET_HEAP_SIZE(){
  if (!kheap){
    return 0;
  }

  return (kheap->heap_end - kheap->heap_start);
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
  new_heap->max_size = 0x10000000;
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

void extend_heap(){
  
    // If we have room, treat the next 4k block as an existing used
    // block and "free it". If there is a free block at the end of the
    // current heap, they will coalesce; otherwise the new 4k page will
    // just be added at the end of the linked list
    // This should page fault but then return here

    // Get pointer to next page, set as as one "used" block
    void* new_mem = GET_DATA((header_t*)kheap->heap_end);
    SET_HDR(new_mem, DATA_SIZE(PAGE_SIZE), USED_FLAG);

    // Now free that block as if it were part of the heap
    // It is contiguous w/ the existing heap, so this should coalesce
    // if the last block in the heap is free
    kfree(new_mem, kheap);

    // Update heap end
    kheap->heap_end += PAGE_SIZE;
}

void* kalloc(uint32_t size, uint16_t align, heap_t* heap){

  if (0 == size){
    printf("WARNING: Do not call kalloc with 0 size; defaults to 8 bytes\n");
  }

  // Dynamic allocation should be at least 8-bytes
  size = (size > 8) ? size : 8;

  free_hdr_t* free_itr = heap->freelist_head;
  while(free_itr && free_itr->header.size < size){
    free_itr = free_itr->freelist_data.next;
  }

  // If we reach the end of the list w/o finding a block
  if (!free_itr){

    // If we have no more room, fail alloc and return 0
    if ( (GET_HEAP_SIZE() + PAGE_SIZE) > kheap->max_size){
      printf("Cannot extend heap; size requested is larger than max\n");
      return 0x0;
    }

    extend_heap();

    // Now we extended the heap, recurse to try again
    // This works even for 4K+ chunks, it will repeatedly re-alloc
    return kalloc(size, align, heap);
  }

  // Get data ptr
  void* ptr = GET_DATA(&(free_itr->header));

  // The free block includes linked list info, which we need
  freelist_data_t freelist_links = free_itr->freelist_data;

  // Also save original block size
  uint32_t block_remainder = free_itr->header.size - size;

  // Setup the current block
  SET_HDR_FTR(ptr, size, USED_FLAG);

  // breakpoint();

  //breakpoint();
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
    new_links->next = 0x0;  // Clear out any data from used block
    new_links->prev = 0x0;  // Clear out any data from used block
    LINK_UP_FREELIST(new_links);

    //breakpoint();
    if (free_itr == heap->freelist_head){
      heap->freelist_head = FREE_HDR_FROM_LIST(new_links);
    }
  } else {
    // breakpoint();
    REMOVE_FROM_FREELIST(&(free_itr->freelist_data));
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
  if (!next_ptr || ((uint32_t)next_ptr >= kheap->heap_end)){
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
    if (next_ptr_links->prev || next_ptr_links->next){
      if (cur_ptr_links->prev || cur_ptr_links->next){
	// If cur ptr already has freelist links, unlink next ptr
	REMOVE_FROM_FREELIST(next_ptr_links);
      } else {
	// If ptr has no free links, steal them from next ptr
	*cur_ptr_links = *next_ptr_links;
	LINK_UP_FREELIST(cur_ptr_links);	
      }
    }

    // If next ptr was freelist_head, assume that role
    if (FREE_HDR_FROM_LIST(next_ptr_links) == kheap->freelist_head){
      kheap->freelist_head = FREE_HDR_FROM_LIST(cur_ptr_links);
    }

    // Indicate that we performed some coalesce
    performed_coalesce++;
  }

  return performed_coalesce;
}

void kfree(void* ptr, heap_t* heap){

  // Make sure ptr is non-null and allocated
  if (!ptr || IS_FREE(GET_HDR(ptr))){
    return;
  }

  // Mark the individual block as free
  MARK_FREE(ptr);

  // Clear out garbage data where the freelist links will go
  ZERO_FREELIST_LINKS(ptr);

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




// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// TESTING
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------


static void print_block(header_t* hdr){

  // Get necessary data
  void* ptr = GET_DATA(hdr);
  footer_t* ftr = GET_FTR(ptr);
  const char* block_free = IS_FREE(hdr) ? "free" : "used";
  uint32_t block_size = GET_SIZE(ptr);
  freelist_data_t* free_links = IS_FREE(hdr) ? (freelist_data_t*)ptr : 0x0;

  // "pretty" print
  printf("Hdr addr: %x -- Ptr addr: %x -- Ftr addr: %x\n", (uint32_t)hdr, (uint32_t)ptr, (uint32_t)ftr);
  printf("Block is %s -- Contains %d bytes\n", block_free, block_size);
  if (free_links){
    uint32_t next = (uint32_t)free_links->next;
    uint32_t prev = (uint32_t)free_links->prev;
    printf("freelist->next = %x -- freellist->prev = %x\n", next, prev);
  }

}

static void print_freelist(heap_t* heap){

  printf("---- Printing Freelist ----\n");
  free_hdr_t* free_itr = heap->freelist_head;
  while (free_itr){
    print_block(&free_itr->header);
    free_itr = free_itr->freelist_data.next;
  }
}

static uint32_t count_free_blocks(heap_t* heap){
  uint32_t num_free_blocks = 0;
  
  free_hdr_t* free_itr = heap->freelist_head;
  while (free_itr){
    num_free_blocks++;
    free_itr = free_itr->freelist_data.next;
  }

  return num_free_blocks;
}

static void print_heap_change(char* op, uint32_t* ptr, free_hdr_t* freelist_head){
  printf("%s, addr = %x -- freelist_head = %x\n", op, (uint32_t)ptr, (uint32_t)freelist_head);
}


// **************************************************
// **** DANGER: THIS WILL RESET THE ENTIRE HEAP ****
// **** USE ONLY FOR TESTING AND DEBUGGING ****
// **************************************************
static void clear_heap(uint32_t safety) {

  // Just make sure I don't accidentally call this
  if (safety != 8675309){
    return;
  }
  
  // Zero out the entire heap section
  void* heap = (void*)kheap->heap_start;
  uint32_t size = kheap->heap_end - kheap->heap_start;
  uint32_t val = 0x0;
  memset(heap, val, size);

  // Set up the heap for use
  uint32_t avail_space = DATA_SIZE(size);
  SET_HDR_FTR(GET_DATA((header_t*)kheap->heap_start), avail_space, FREE_FLAG);
  kheap->freelist_head = (free_hdr_t*)kheap->heap_start;
  kheap->freelist_head->freelist_data.next = 0x0;
  kheap->freelist_head->freelist_data.prev = 0x0;

}

void TEST_alloc(){

  // Make sure we have a fresh heap
  clear_heap(8675309);

  // First available addr to allocate
  uint32_t base_addr = (uint32_t)GET_DATA(&kheap->freelist_head->header);

  // Do a single allocation
  void* ptr = kalloc(32, 0, kheap);
  uint32_t ptr_addr = (uint32_t)ptr;
  uint32_t expected_addr = base_addr;

  ASSERT_EQ(ptr_addr, expected_addr);
  
  // Do several allocations
  clear_heap(8675309);
  
  for (int i = 0; i < 5; i++){
    void* itr_ptr = kalloc(32, 0, kheap);
    ptr_addr = (uint32_t)itr_ptr;
    expected_addr = base_addr + TOTAL_BLK_SIZE(32)*i;

    ASSERT_EQ(ptr_addr, expected_addr);
  }
  
  // End test and leave the heap clean when we're done
  END_TEST(TEST_alloc);
  clear_heap(8675309);
}

void TEST_free(){
  // Make sure we have a fresh heap
  clear_heap(8675309);

  // Save some data to compare to post-allocation
  uint32_t freelist_head = (uint32_t)kheap->freelist_head;
  uint32_t orig_size = kheap->freelist_head->header.size;

  // First available addr to allocate
  uint32_t base_addr = (uint32_t)GET_DATA(&kheap->freelist_head->header);

  // Do a single allocation, to be freed
  void* ptr = kalloc(32, 0, kheap);
  uint32_t ptr_addr = (uint32_t)ptr;
  uint32_t expected_addr = base_addr;
  uint32_t ptr_size = GET_SIZE(ptr);

  // Make sure the allocation worked
  ASSERT_EQ(ptr_addr, expected_addr);
  ASSERT_EQ(ptr_size, 32);

  // Freelist head should be shifted to past the allocated block
  uint32_t new_freelist_head = freelist_head + TOTAL_BLK_SIZE(32);
  ASSERT_EQ((uint32_t)kheap->freelist_head, new_freelist_head);

  // Now free that pointer, make sure freelist was properly fixed up
  kfree(ptr, kheap);

  ASSERT_EQ((uint32_t)kheap->freelist_head, freelist_head);
  ASSERT_EQ(kheap->freelist_head->header.size, orig_size);
  ASSERT_TRUE(IS_FREE(&(kheap->freelist_head->header)));

  // Now allocate a new block, should be identical to the first
  void* ptr2 = kalloc(32, 0, kheap);
  uint32_t ptr_addr2 = (uint32_t)ptr2;
  uint32_t expected_addr2 = base_addr;
  uint32_t ptr_size2 = GET_SIZE(ptr2);

  ASSERT_EQ(ptr_addr2, expected_addr2);
  ASSERT_EQ(ptr_addr2, ptr_addr);
  ASSERT_EQ(ptr_size2, 32);
  ASSERT_EQ(ptr_size2, ptr_size);

  // End test and leave the heap clean when we're done
  END_TEST(TEST_free);  
  clear_heap(8675309);
}

void TEST_freelist(){
  // Make sure we have a fresh heap
  clear_heap(8675309);

  // Shared comparison data
  uint32_t initial_heap_size = kheap->heap_end - kheap->heap_start;

  // (---0---) Check initial state of the freelist
  header_t* freelist_head0 = &(kheap->freelist_head->header);
  freelist_data_t* free_links0 = &(kheap->freelist_head->freelist_data);
  uint32_t header_size_field0 = freelist_head0->size; // Free block, low bit unset
  ASSERT_EQ((uint32_t)freelist_head0, kheap->heap_start);
  ASSERT_EQ(DATA_SIZE(initial_heap_size), header_size_field0);
  ASSERT_EQ(free_links0->next, 0x0);
  ASSERT_EQ(free_links0->prev, 0x0);

  // (---1---)Allocate a block, check the new freelist head
  void* ptr1 = kalloc(32, 0, kheap);
  header_t* freelist_head1 = &(kheap->freelist_head->header);
  freelist_data_t* free_links1 = &(kheap->freelist_head->freelist_data);
  uint32_t new_size1 = freelist_head1->size; // Free block, low bit unset
  ASSERT_EQ((uint32_t)freelist_head1, ((uint32_t)freelist_head0 + TOTAL_BLK_SIZE(32)));
  ASSERT_EQ((DATA_SIZE(initial_heap_size) - TOTAL_BLK_SIZE(32)), new_size1);
  ASSERT_EQ(free_links1->next, 0x0);
  ASSERT_EQ(free_links1->prev, 0x0);

  // (---2---)Allocate a second block, check the new freelist head
  void* ptr2 = kalloc(32, 0, kheap);
  header_t* freelist_head2 = &(kheap->freelist_head->header);
  freelist_data_t* free_links2 = &(kheap->freelist_head->freelist_data);
  uint32_t new_size2 = freelist_head2->size; // Free block, low bit unset
  ASSERT_EQ((uint32_t)freelist_head2, ((uint32_t)freelist_head1 + TOTAL_BLK_SIZE(32)));
  ASSERT_EQ((DATA_SIZE(initial_heap_size) - TOTAL_BLK_SIZE(32)*2), new_size2);
  ASSERT_EQ(free_links2->next, 0x0);
  ASSERT_EQ(free_links2->prev, 0x0);

  // (---3---) Free second block, coalesce left. Should return to state after first alloc
  kfree(ptr2, kheap);
  header_t* freelist_head3 = &(kheap->freelist_head->header);
  freelist_data_t* free_links3 = &(kheap->freelist_head->freelist_data);
  uint32_t new_size3 = freelist_head3->size; // Free block, low bit unset
  ASSERT_EQ((uint32_t)freelist_head3, (uint32_t)freelist_head1);
  ASSERT_EQ(new_size3, new_size1);
  ASSERT_EQ(free_links3->next, 0x0);
  ASSERT_EQ(free_links3->prev, 0x0);

  // (---4---) Re-allocate a second block, make sure freelist is same as (---2---)
  void* ptr4 = kalloc(32, 0, kheap);
  header_t* freelist_head4 = &(kheap->freelist_head->header);
  freelist_data_t* free_links4 = &(kheap->freelist_head->freelist_data);
  uint32_t new_size4 = freelist_head4->size; // Free block, low bit unset
  ASSERT_EQ((uint32_t)freelist_head4, (uint32_t)freelist_head2);
  ASSERT_EQ(new_size4, new_size2);
  ASSERT_EQ(free_links4->next, 0x0);
  ASSERT_EQ(free_links4->prev, 0x0);

  // (---5---) Free first block, new entry on freelist (and new freelist head)
  header_t* ptr_hdr = GET_HDR(ptr1);
  header_t hdr_copy = *ptr_hdr;  // Save data before freeing
  free_hdr_t* orig_free_head = kheap->freelist_head;
  kfree(ptr1, kheap);

  header_t* freelist_head5 = &(kheap->freelist_head->header);
  freelist_data_t* free_links5 = &(kheap->freelist_head->freelist_data);
  uint32_t new_size5 = freelist_head5->size; // Free block, low bit unset
  ASSERT_EQ((uint32_t)freelist_head5, kheap->heap_start); // Used block was at start of heap
  ASSERT_EQ((uint32_t)freelist_head5, (uint32_t)ptr_hdr);
  ASSERT_EQ(new_size5, (hdr_copy.size & (~0x1)));
  ASSERT_EQ((uint32_t)free_links5->next, (uint32_t)orig_free_head);
  ASSERT_EQ(free_links5->prev, 0x0);

  // Ensure next free block is the last (remainder of the heap)
  free_hdr_t* next_free5 = free_links5->next;
  header_t* next_freelist_head5 = &(next_free5->header);
  ASSERT_EQ((uint32_t)next_freelist_head5, (uint32_t)freelist_head4);
  ASSERT_EQ(next_freelist_head5->size, new_size4);
  ASSERT_EQ(next_free5->freelist_data.next, 0x0);
  ASSERT_EQ((uint32_t)next_free5->freelist_data.prev, (uint32_t)FREE_HDR_FROM_LIST(free_links5));

  // ------------------------------------------
  // CLEAR HEAP, CLEAN SLATE FOR UPCOMING TESTS
  // ------------------------------------------
  clear_heap(8675309);
  // ------------------------------------------
  // ------------------------------------------

  // (---6---) Testing multiple free blocks
  void* used_blocks[5];
  header_t* used_hdrs[5]; // Save for tests
  for (int i = 0; i < 5; i++){
    used_blocks[i] = kalloc(32, 0, kheap);
    used_hdrs[i] = GET_HDR(used_blocks[i]);
  }

  // Validate initial free block (heap remainder) assumptions
  header_t* freelist_head6 = &(kheap->freelist_head->header);
  freelist_data_t* free_links6 = &(kheap->freelist_head->freelist_data);
  ASSERT_EQ((uint32_t)freelist_head6, (kheap->heap_start + TOTAL_BLK_SIZE(32)*5));
  ASSERT_EQ(freelist_head6->size, (DATA_SIZE(initial_heap_size) - TOTAL_BLK_SIZE(32)*5));
  ASSERT_EQ(free_links6->next, 0x0);
  ASSERT_EQ(free_links6->prev, 0x0);

  // Now free 2 blocks, which don't coalesce
  kfree(used_blocks[1], kheap);
  kfree(used_blocks[3], kheap);

  // Should have 3 free (used_blocks[1], used_blocks[3], heap remainder)
  ASSERT_EQ(count_free_blocks(kheap), 3);

  // We freed used_blocks[3] last, it should be the freelist head
  ASSERT_EQ((uint32_t)kheap->freelist_head, (uint32_t)used_hdrs[3]);
  ASSERT_EQ(kheap->freelist_head->header.size, 32);

  // Next free block should be from used_blocks[1]
  free_hdr_t* next_free = kheap->freelist_head->freelist_data.next;
  ASSERT_EQ((uint32_t)next_free, (uint32_t)used_hdrs[1]);
  ASSERT_EQ(next_free->header.size, 32);

  // Last block should be the heap remainder
  free_hdr_t* last_free = next_free->freelist_data.next;
  ASSERT_EQ((uint32_t)last_free, (uint32_t)freelist_head6);
  ASSERT_EQ(last_free->header.size, (DATA_SIZE(initial_heap_size) - TOTAL_BLK_SIZE(32)*5));
  ASSERT_EQ(last_free->freelist_data.next, 0x0);


  // (---7---) Use the first free block on the list, then the second
  kalloc(32, 0, kheap);
  header_t* freelist_head7 = &(kheap->freelist_head->header);
  freelist_data_t* free_links7 = &(kheap->freelist_head->freelist_data);
  ASSERT_EQ((uint32_t)freelist_head7, (uint32_t)used_hdrs[1]);
  ASSERT_EQ(free_links7->prev, 0x0);

  // Should have 2 free (used_blocks[1], heap remainder)
  ASSERT_EQ(count_free_blocks(kheap), 2);

  // (---8---) Use the second block
  header_t* last_hdr8 = &(free_links7->next->header);
  kalloc(32, 0, kheap);
  header_t* freelist_head8 = &(kheap->freelist_head->header);
  freelist_data_t* free_links8 = &(kheap->freelist_head->freelist_data);
  ASSERT_EQ((uint32_t)freelist_head8, (uint32_t)last_hdr8);
  ASSERT_EQ(free_links8->next, 0x0);
  ASSERT_EQ(free_links8->prev, 0x0);

  // Should have 1 free (heap remainder)
  ASSERT_EQ(count_free_blocks(kheap), 1);

  // End test and leave the heap clean when we're done
  END_TEST(TEST_freelist);  
  clear_heap(8675309);
}

void TEST_coalesce(){
  // Make sure we have a fresh heap
  clear_heap(8675309);


  // Shared comparison data
  uint32_t initial_heap_size = kheap->heap_end - kheap->heap_start;

  // (---0---) Check initial state of the freelist
  header_t* freelist_head0 = &(kheap->freelist_head->header);
  freelist_data_t* free_links0 = &(kheap->freelist_head->freelist_data);
  uint32_t header_size_field0 = freelist_head0->size; // Free block, low bit unset
  ASSERT_EQ((uint32_t)freelist_head0, kheap->heap_start);
  ASSERT_EQ(DATA_SIZE(initial_heap_size), header_size_field0);
  ASSERT_EQ(free_links0->next, 0x0);
  ASSERT_EQ(free_links0->prev, 0x0);
  ASSERT_EQ(count_free_blocks(kheap), 1);

  // (--1--) Alloc a block then free it (coalesce with heap remainder)
  void* ptr1 = kalloc(32, 0, kheap);
  kfree(ptr1, kheap);

  // Freeblock metrics should be the same as before
  header_t* freelist_head1 = &(kheap->freelist_head->header);
  uint32_t header_size_field1 = freelist_head1->size; // Free block, low bit unset
  ASSERT_EQ((uint32_t)freelist_head1, kheap->heap_start);
  ASSERT_EQ((uint32_t)freelist_head1, (uint32_t)freelist_head0);
  ASSERT_EQ(DATA_SIZE(initial_heap_size), header_size_field1);
  ASSERT_EQ(count_free_blocks(kheap), 1);

  // (--2--) Alloc 2 blocks, free the first block - no coalesce
  void* ptr2_1 = kalloc(32, 0, kheap);
  void* ptr2_2 = kalloc(32, 0, kheap);
  // Get heap remainder header before freeing ptr
  header_t* heap_remainder = &(kheap->freelist_head->header);
  kfree(ptr2_1, kheap);

  ASSERT_EQ(count_free_blocks(kheap), 2);
  header_t* freelist_head2 = &(kheap->freelist_head->header);
  freelist_data_t* free_links2 = &(kheap->freelist_head->freelist_data);
  uint32_t header_size_field2 = freelist_head2->size; // Free block, low bit unset
  ASSERT_EQ((uint32_t)freelist_head2, kheap->heap_start);
  ASSERT_EQ(32, header_size_field2);
  ASSERT_EQ(free_links2->prev, 0x0);
  ASSERT_EQ((uint32_t)free_links2->next, (uint32_t)heap_remainder);

  // (--3--) Free middle alloc'd block, coalesce left and right
  kfree(ptr2_2, kheap);

  ASSERT_EQ(count_free_blocks(kheap), 1);
  header_t* freelist_head3 = &(kheap->freelist_head->header);
  uint32_t header_size_field3 = freelist_head3->size; // Free block, low bit unset
  ASSERT_EQ((uint32_t)freelist_head3, kheap->heap_start);
  ASSERT_EQ((uint32_t)freelist_head3, (uint32_t)freelist_head0);
  ASSERT_EQ(DATA_SIZE(initial_heap_size), header_size_field3);
  
  
  
  // End test and leave the heap clean when we're done
  END_TEST(TEST_freelist);  
  clear_heap(8675309);
}

void TEST_multiple_page_heap(){
  // Make sure we have a fresh heap
  clear_heap(8675309);


  // Shared comparison data
  uint32_t initial_heap_size = kheap->heap_end - kheap->heap_start;
  uint32_t base_addr = (uint32_t)GET_DATA(&kheap->freelist_head->header);

  // (---0---) Check initial state of the freelist
  header_t* freelist_head0 = &(kheap->freelist_head->header);
  freelist_data_t* free_links0 = &(kheap->freelist_head->freelist_data);
  uint32_t header_size_field0 = freelist_head0->size; // Free block, low bit unset
  ASSERT_EQ((uint32_t)freelist_head0, kheap->heap_start);
  ASSERT_EQ(DATA_SIZE(initial_heap_size), header_size_field0);
  ASSERT_EQ(free_links0->next, 0x0);
  ASSERT_EQ(free_links0->prev, 0x0);
  ASSERT_EQ(count_free_blocks(kheap), 1);

  // (--1--) Alloc block that extends into next page

  // Start by allocating three 1K chunks
  for (int i = 0; i < 3; i++){
    kalloc(1024, 0, kheap);
  }

  // Now allocate a 2K chunk (extending into next page)
  void* ptr = kalloc(2048, 0, kheap);

  // Allocation should have worked
  uint32_t ptr_addr = (uint32_t)ptr;
  uint32_t expected_addr = base_addr + (3*TOTAL_BLK_SIZE(1024));
  ASSERT_EQ(ptr_addr, expected_addr);

  // Next block should be 3K free block
  void* next_ptr = NEXT_BLOCK(ptr);
  header_t* next_hdr = GET_HDR(next_ptr);
  ASSERT_EQ(IS_FREE(next_hdr), 1);
  ASSERT_EQ(GET_SIZE(next_ptr), 0xBB0);
  
  
  // End test and leave the heap clean when we're done
  END_TEST(TEST_multi_page_heap);
  clear_heap(8675309);
}

void TEST_kheap(){

  TEST_alloc();
  TEST_free();
  TEST_freelist();
  TEST_coalesce();
  TEST_multiple_page_heap();


  // Clear heap at the end, just in case
  clear_heap(8675309);

}
