#include "kernel/pmm.h"
#include "common/inline_assembly.h"

// Page Fault Handler
void page_fault_handler(uint32_t faulting_addr, uint32_t error_code){


  if ((error_code & 0x1) == 0){
      uint32_t pd_index = faulting_addr >> 22;
      uint32_t pt_index = ((faulting_addr >> 12) & 0x3FF);

      // Get page directory virtual address (via recursive mapping)
      page_directory_t* page_directory = (page_directory_t*)0xFFFFF000;

      // Get page table physical and virtual address
      uint32_t page_table_phys = (uint32_t)page_directory->page_tables[pd_index];      
      page_table_t* page_table = (page_table_t*)((uint32_t*)0xFFC00000 + (0x400 * pd_index));

      // If page table is not present, create it
      if ((page_table_phys & 0x1) == 0){
	// Grab an available physical frame (returns index, so mult * 1000)
	//breakpoint();
	uint32_t new_table_frame = first_frame() * 0x1000;
	// breakpoint();

	// Add new page table to the page directory
	page_directory->page_tables[pd_index] = (page_table_t*)(new_table_frame | 0x3);
      }

      // Now add the page entry
      page_t* newpage = &(page_table->pages[pt_index]);
      if (!newpage->present && !newpage->frame){
	alloc_frame(newpage, 1, 1);
      }

  }
  
}
