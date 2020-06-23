#include "kernel/vmm.h"
#include "kernel/pmm.h"


// ---------------------------------------------------------
// Paging Logic
// ---------------------------------------------------------

void initialize_paging(){

  /* // Set up physical frames */
  /* num_frames = PHYSICAL_MEM_SIZE / 0x1000; */
  /* frames = (uint32_t*)kmalloc(FRAME_BITSET_FROM_ADDR(num_frames)); */
  /* memset(frames, 0, FRAME_BITSET_FROM_ADDR(nframes)); */

  /* // Create the page directory */
  /* kernel_directory = (page_directory_t*)kmalloc(sizeof(page_directory_t)); */
  /* memset(kernel_directory, 0, sizeof(page_directory_t)); */
  /* current_directory = kernel_directory; */

  /* // Identity-map the kernel */
  /* int i = 0; */
  /* while(i < placement_address){ */
  /*   alloc_frame(get_page(i, 1, kernel_directory), 0, 0); */
  /*   i += 0x1000; */
  /* } */

  /* // Enable paging */
  /* switch_page_directory(kernel_directory); */
}

// @TODO implementation
// Should really call raw assemly function, avoid inline
void switch_page_directory(page_directory_t* page_dir){

  /* current_directory = dir; */
  /* asm volatile("mov %0, %%cr3":: "r"(&dir->tablesPhysical)); */
  /* u32int cr0; */
  /* asm volatile("mov %%cr0, %0": "=r"(cr0)); */
  /* cr0 |= 0x80000000; // Enable paging! */
  /* asm volatile("mov %0, %%cr0":: "r"(cr0)); */
}

// @TODO implementation
page_t* get_page(uint32_t address, int create, page_directory_t* dir){

  return 0;

}
