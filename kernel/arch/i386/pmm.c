#include "kernel/pmm.h"
#include "string.h"
#include "common/inline_assembly.h"

// ---------------------
// Global Frame Data
// ---------------------
extern uint32_t pmm_num_frames;
extern uint32_t* pmm_frames;


uint32_t num_frames = 0;
uint32_t* frames = (uint32_t*)0x0;


// ----------------------------------------
// Frame Allocation Helpers
// ----------------------------------------

static void set_frame(uint32_t frame_addr){

  // We divide memory into 4K chunks
  // 0x0000 is frame 0, 0x1000 is frame 1...
  uint32_t frame_number = frame_addr / 0x1000;

  // Get index (i.e. which 32-bit bitfield)
  // Get the offset (which bit in bitfield corresponds to frame_number)
  // Turn that num into a mask (e.g offset == 4 --> 0b00010000
  uint32_t index = FRAME_BITSET_FROM_ADDR(frame_number);
  uint32_t offset = FRAME_BIT_OFFSET_FROM_ADDR(frame_number);
  uint32_t mask = (0x1 << offset);
  frames[index] |= mask;
}

// Same as set_frame, but clear the bit
static void clear_frame(uint32_t frame_addr){

  uint32_t frame_number = frame_addr / 0x1000;

  // Mask is a zero in that slot, and we AND it, e.g. 0b11101111
  uint32_t index = FRAME_BITSET_FROM_ADDR(frame_number);
  uint32_t offset = FRAME_BIT_OFFSET_FROM_ADDR(frame_number);
  uint32_t mask = ~(0x1 << offset);  // Set bit, then flip all bits
  frames[index] &= mask;
}

// Check if a frame is allocated
static uint32_t test_frame(uint32_t frame_addr){

  uint32_t frame_number = frame_addr / 0x1000;

  uint32_t index = FRAME_BITSET_FROM_ADDR(frame_number);
  uint32_t offset = FRAME_BIT_OFFSET_FROM_ADDR(frame_number);
  uint32_t mask = (0x1 << offset);
  return frames[index] & mask;
}

// @TODO this is "first fit", maybe implement next-fit
uint32_t first_frame(){

  //  breakpoint();

  // Use macro for consistency - just get the number of 32-bit bitfields
  uint32_t num_bitsets = FRAME_BITSET_FROM_ADDR(num_frames);
  
  uint32_t i, j;
  for (i = 0; i < num_bitsets; i++){

    // All bits set means none available in this set
    if (frames[i] != 0xFFFFFFFF){
      for (j = 0; j < 32; j++){
	// If bit is not set, this frame is free
	uint32_t mask = 0x1 << j;
	if ((frames[i] & mask) == 0){
	  // Distance into bitset list + distance into this bitset
	  uint32_t res = (i*32) + j;
	  set_frame(res * 0x1000);
	  return res;
	}
      }
    }
  }

  return (uint32_t)-1;
}

// ----------------------------------------
// Page Allocation & De-Allocation
// ----------------------------------------

void alloc_frame(page_t* page, int is_kernel, int is_writeable){

  // If page is allocated, don't re-allocate
  if (page->frame != 0){
    return;
  }

  uint32_t frame_index = first_frame();
  
  // Sanity check - @TODO error here
  if (frame_index == (uint32_t)-1){
    return;
  }
  
  // Mark this physical frame as allocated
  set_frame(frame_index * 0x1000);

  // Set page attributes
  page->present = 1;
  page->rw = (is_writeable) ? 1 : 0;
  page->user = (is_kernel) ? 0 : 1;
  page->frame = frame_index;
}

void free_frame(page_t* page){

  // If page is null or has no frame, return
  if (!page || (page->frame == 0)){
    return;
  }

  // Mark physical page as available
  // Clear our this page's frame
  clear_frame(page->frame * 0x1000);
  page->frame = 0x0;
}


// ----------------
// Setup
// ----------------

void setup_pmm(){

  
  frames = pmm_frames;  
  num_frames = pmm_num_frames;

  // Set first 4Mb as unavailable (kernel page table)
  // 4Mb = 1024 frames, at 1 bit per frame
  // 1024 bits / sizeof(byte to set) = 128
  memset(frames, 0xff, 128);

  // Set the rest as free: 124MB
  memset(&frames[32], 0x0, 3968);
}
