#include "kernel/paging.h"
#include "kernel/kheap.h"
#include "string.h"

#define PHYSICAL_MEM_SIZE 0x2000000 // 32MB

// ----------------------------------------
// Data
// ----------------------------------------

// Defined in kheap.h
extern uint32_t placement_address;

// The kernel's page directory
page_directory_t *kernel_directory=0;

// The current page directory;
page_directory_t *current_directory=0;

uint32_t num_frames;
uint32_t* frames;

// ----------------------------------------
// Bitset Helper Macros
// ----------------------------------------

#define INDEX_FROM_BIT(a)  ((a)/(8*4))
#define OFFSET_FROM_BIT(a) ((a)%(8*4))


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
  uint32_t index = INDEX_FROM_BIT(frame_number);
  uint32_t offset = OFFSET_FROM_BIT(frame_number);
  uint32_t mask = (0x1 << offset);
  frames[index] |= mask;
}

// Same as set_frame, but clear the bit
static void clear_frame(uint32_t frame_addr){

  uint32_t frame_number = frame_addr / 0x1000;

  // Mask is a zero in that slot, and we AND it, e.g. 0b11101111
  uint32_t index = INDEX_FROM_BIT(frame_number);
  uint32_t offset = OFFSET_FROM_BIT(frame_number);
  uint32_t mask = ~(0x1 << offset);  // Set bit, then flip all bits
  frames[index] &= mask;
}

/*
// Comment until it's used; shut up warning 
static uint32_t test_frame(uint32_t frame_addr){

  uint32_t frame_number = frame_addr / 0x1000;

  uint32_t index = INDEX_FROM_BIT(frame_number);
  uint32_t offset = OFFSET_FROM_BIT(frame_number);
  uint32_t mask = (0x1 << offset);
  return frames[index] & mask;
}
*/

static uint32_t first_frame(){

  // Use macro for consistency - just get number of 32-bit bitfields
  uint32_t num_bitsets = INDEX_FROM_BIT(num_frames);
  
  uint32_t i, j;
  for (i = 0; i < num_bitsets; i++){

    // All bits set means none available in this set
    if (frames[i] != 0xFFFFFFFF){
      for (j = 0; j < 32; j++){
	// If bit is not set, this frame is free
	uint32_t mask = 0x1 << j;
	if ((frames[i] & mask) == 0){
	  // Distance into bitset list + distance into this bitset
	  return (i*32) + j;
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




// ---------------------------------------------------------
// Paging Logic
// ---------------------------------------------------------

void initialize_paging(){

  // Set up physical frames
  num_frames = PHYSICAL_MEM_SIZE / 0x1000;
  frames = (uint32_t*)kmalloc(INDEX_FROM_BIT(num_frames));
  memset(frames, 0, INDEX_FROM_BIT(num_frames));

  // Create the page directoryc
  kernel_directory = (page_directory_t*)kmalloc_a(sizeof(page_directory_t));
  memset(kernel_directory, 0, sizeof(page_directory_t));
  current_directory = kernel_directory;

  // Identity-map the kernel
  uint32_t i = 0;
  while(i < placement_address){
    alloc_frame(get_page(i, 1, kernel_directory), 0, 0);
    i += 0x1000;
  }

  // Register page fault handler
  // register_interrupt_handler(14, page_fault);

  // Enable paging
  switch_page_directory(kernel_directory);
}

void switch_page_directory(page_directory_t* page_dir){

  current_directory = page_dir;
  asm volatile("mov %0, %%cr3":: "r"(&page_dir->page_tables_physical));
  uint32_t cr0;
  asm volatile("mov %%cr0, %0": "=r"(cr0));
  cr0 |= 0x80000000; // Enable paging!
  asm volatile("mov %0, %%cr0":: "r"(cr0));
}

page_t* get_page(uint32_t address, int create, page_directory_t* dir){

  // Convert to page index
  address /= 0x1000;

  uint32_t table_index = address / 1024;
  if (dir->page_tables[table_index]){
    return &dir->page_tables[table_index]->pages[address%1024];
  } else if (create) {
    uint32_t tmp = 0;
    dir->page_tables[table_index] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &tmp);
    memset(dir->page_tables[table_index], 0, sizeof(page_table_t));
    dir->page_tables_physical[table_index] = tmp | 0x7; // Present, RW, US
    return &dir->page_tables[table_index]->pages[address%1024];
  } else {
    return 0;
  }

}
