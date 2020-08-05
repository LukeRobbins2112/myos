#include "kernel/vmm.h"
#include "kernel/pmm.h"

// -----------------------------------
// Page Directory -- defined in boot.S
// -----------------------------------
extern page_directory_t* boot_page_directory;
extern uint32_t kernel_end;

// ----------------------------------------------------
// Memory Manipulation helpers
// ----------------------------------------------------

void * get_physaddr(void * virtualaddr)
{
  uint32_t pd_index = (uint32_t)virtualaddr >> 22;
  uint32_t pt_index = (uint32_t)virtualaddr >> 12 & 0x03FF;

  // Get page directory, check whether the PD entry is present.
  uint32_t * pd = (uint32_t *)0xFFFFF000;
  uint32_t page_table_phys = (uint32_t)((page_directory_t*)pd)->page_tables[pd_index];
  if ((page_table_phys & 0x1) == 0){
    return 0;
  }

  // Get page table, check whether the PT entry is present.  
  uint32_t * pt = ((uint32_t *)0xFFC00000) + (0x400 * pd_index);
  page_t pte = ((page_table_t*)pt)->pages[pt_index];
  if (!pte.present){
    return 0;
  }
  
  
  return (void *)((pt[pt_index] & ~0xFFF) + ((uint32_t)virtualaddr & 0xFFF));
}

// ---------------------------------------------------------
// Paging Logic
// ---------------------------------------------------------

void initialize_paging(){

  // Set up initial kernel heap page table
  //page_table_t* kheap_page_table = (page_table_t*)0xD0000000;
  
  

}

// @TODO implementation
// Should really call raw assemly function, avoid inline
//void switch_page_directory(page_directory_t* page_dir){

  /* current_directory = dir; */
  /* asm volatile("mov %0, %%cr3":: "r"(&dir->tablesPhysical)); */
  /* u32int cr0; */
  /* asm volatile("mov %%cr0, %0": "=r"(cr0)); */
  /* cr0 |= 0x80000000; // Enable paging! */
  /* asm volatile("mov %0, %%cr0":: "r"(cr0)); */
//}

// @TODO implementation
/* page_t* get_page(uint32_t vaddr, int create, page_directory_t* dir){ */

/*   uint32_t pd_index = (uint32_t)vaddr >> 22; */
/*   uint32_t pt_index = (uint32_t)vaddr >> 12 & 0x03FF; */

/*   page_table_t* page_table = boot_page_directory->page_tables[pd_index]; */
/*   uint32_t page_table_present = ((uint32_t)page_table & 0x1); */
/*   if (!page_table_present){ */
/*     if (create){ */
/*       // @TODO finished after heap code */
/*       // page_table_t new_table = (page_table_t*)kmalloc_aligned(sizeof(page_table_t)); */
/*       // memset(*new_table, 0x0, sizeof(page_table_t)); */
/*       // new_table |= 0x3; // Present, RW */
/*       // boot_page_directory[pd_index] = new_table; */
/*       // return &boot_page_directory->page_tables[pd_index]->pages[pt_index]; */
/*     } */
/*   } else { */
/*     // Page Table present, get page entry and return as is */
/*     // Return even if not allocated - don't allocate it */
/*     page_t* page = &page_table->pages[pt_index]; */
/*     return page; */
/*   } */
  
  
/* } */
