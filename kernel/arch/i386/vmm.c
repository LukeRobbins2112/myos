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

void * get_pt_physaddr(void* virtualaddr){
  uint32_t pd_index = get_pd_index((uint32_t)virtualaddr);

  // Get page directory, check whether the PD entry is present.
  page_directory_t * pd = get_page_directory();
  uint32_t page_table_phys = (uint32_t)(pd->page_tables[pd_index]);

  return (void *)page_table_phys;
}

void* get_pt_virtaddr(uint32_t pd_index){
  return (void*)(((uint32_t *)0xFFC00000) + (0x400 * pd_index));
}

void * get_physaddr(void * virtualaddr)
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
  uint32_t offset = ((unsigned long)virtualaddr & 0xFFF);
  return (void *)(base + offset);
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
