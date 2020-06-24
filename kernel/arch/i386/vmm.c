#include "kernel/vmm.h"
#include "kernel/pmm.h"

// -----------------------------------
// Page Directory -- defined in boot.S
// -----------------------------------
extern page_directory_t* boot_page_directory;
extern uint32_t kernel_end;


// ---------------------------------------------------------
// Paging Logic
// ---------------------------------------------------------

void initialize_paging(){

  // Set up initial kernel heap page table
  page_table_t* kheap_page_table = (page_table_t*)0xD0000000;
  
  

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
page_t* get_page(uint32_t vaddr, int create, page_directory_t* dir){

  uint32_t pd_index = (uint32_t)vaddr >> 22;
  uint32_t pt_index = (uint32_t)vaddr >> 12 & 0x03FF;

  page_table_t* page_table = boot_page_directory->page_tables[pd_index];
  uint32_t page_table_present = ((uint32_t)page_table & 0x1);
  if (!page_table_present){
    if (create){
      // @TODO finished after heap code
      // page_table_t new_table = (page_table_t*)kmalloc_aligned(sizeof(page_table_t));
      // memset(*new_table, 0x0, sizeof(page_table_t));
      // new_table |= 0x3; // Present, RW
      // boot_page_directory[pd_index] = new_table;
      // return &boot_page_directory->page_tables[pd_index]->pages[pt_index];
    }
  } else {
    // Page Table present, get page entry and return as is
    // Return even if not allocated - don't allocate it
    page_t* page = &page_table->pages[pt_index];
    return page;
  }
  
  
}
