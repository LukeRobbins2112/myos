#include "kernel/vmm.h"
#include "kernel/pmm.h"
#include <common/inline_assembly.h>

// -----------------------------------
// Page Directory -- defined in boot.S
// -----------------------------------
extern page_directory_t* boot_page_directory;
extern uint32_t kernel_end;

// ----------------------------------------------------
// Memory Manipulation helpers
// ----------------------------------------------------

page_directory_t* get_page_directory(){
  return (page_directory_t *)0xFFFFF000;
}

uint32_t get_pd_index(uint32_t addr){
  return addr >> 22;
}

uint32_t get_pt_index(uint32_t addr){
  return ((addr >> 12) & 0x3FF);
}

void * get_pt_physaddr(uint32_t virtualaddr){
  uint32_t pd_index = get_pd_index(virtualaddr);

  // Get page directory, check whether the PD entry is present.
  page_directory_t * pd = get_page_directory();
  uint32_t page_table_phys = (uint32_t)(pd->page_tables[pd_index]);

  return (void *)page_table_phys;
}

void* get_pt_virtaddr(uint32_t pd_index){
  return (void*)(((uint32_t *)0xFFC00000) + (0x400 * pd_index));
}

void * get_physaddr(uint32_t virtualaddr)
{
  uint32_t pd_index = get_pd_index(virtualaddr);
  uint32_t pt_index = get_pt_index(virtualaddr);

  // Get page table, make sure it's present
  uint32_t page_table_phys = (uint32_t)get_pt_physaddr(virtualaddr);
  if ((page_table_phys & 0x1) == 0){
    return 0;
  }

  // Get page table, check whether the PT entry is present.  
  page_table_t * pt = (page_table_t *)get_pt_virtaddr(pd_index);
  page_t pte = pt->pages[pt_index];
  if (!pte.present){
    return 0;
  }

  uint32_t base = (pte.frame << 12);
  uint32_t offset = ((uint32_t)virtualaddr & 0xFFF);
  return (void *)(base + offset);
}

// ---------------------------------------------------------
// Paging Logic
// ---------------------------------------------------------

page_t* get_page(uint32_t vaddr, int create){

  uint32_t pd_index = get_pd_index(vaddr);
  uint32_t pt_index = get_pt_index(vaddr);

  // Get page directory virtual address (via recursive mapping)
  page_directory_t* page_directory = get_page_directory();
  
  // Get page table physical and virtual address
  uint32_t page_table_phys = (uint32_t)get_pt_physaddr(vaddr);
  page_table_t* page_table = (page_table_t*)get_pt_virtaddr(pd_index);

  uint32_t page_table_present = page_table_phys & 0x1;
  if (!page_table_present){
    if (create){
      // Grab an available physical frame (returns index, so mult * 1000)
      uint32_t new_table_frame = first_frame() * 0x1000;
      //breakpoint();
      
      // Add new page table to the page directory
      page_directory->page_tables[pd_index] = (page_table_t*)(new_table_frame | 0x3);
    } else {
      return 0; // PT not present; not creating
    }
  }
  
  // Page Table present, get page entry and return as is
  // Return even if not allocated - don't allocate it
  page_t* page = &(page_table->pages[pt_index]);
  return page;
  
}
