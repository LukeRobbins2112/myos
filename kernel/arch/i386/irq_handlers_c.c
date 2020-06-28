// IRQ C handlers, called from irq_handlers_as

#include "common/inline_assembly.h"
#include "kernel/PIC.h"
#include "kernel/pmm.h"


#define PIC_EOI		0x20		/* End-of-interrupt command code */


static void PIC_sendEOI(unsigned char irq)
{
  if(irq >= 8)
    outb(PIC2_COMMAND, PIC_EOI);
  
  outb(PIC1_COMMAND, PIC_EOI);
}


// Individual Interrupt Service ROutines
void irq0_handler(void) {
  PIC_sendEOI(0);
}
 
void irq1_handler(void) {
  PIC_sendEOI(1);
}
 
void irq2_handler(void) {
  PIC_sendEOI(2);
}
 
void irq3_handler(void) {
  PIC_sendEOI(3);
}
 
void irq4_handler(void) {
  PIC_sendEOI(4);
}
 
void irq5_handler(void) {
  PIC_sendEOI(5);
}
 
void irq6_handler(void) {
  PIC_sendEOI(6);
}
 
void irq7_handler(void) {
  PIC_sendEOI(7);
}
 
void irq8_handler(void) {
  PIC_sendEOI(8);
}
 
void irq9_handler(void) {
  PIC_sendEOI(9);
}
 
void irq10_handler(void) {
  PIC_sendEOI(10);
}
 
void irq11_handler(void) {
  PIC_sendEOI(11);
}
 
void irq12_handler(void) {
  PIC_sendEOI(12);
}
 
void irq13_handler(void) {
  PIC_sendEOI(13);
}
 
void irq14_handler(void) {
  PIC_sendEOI(14);
}
 
void irq15_handler(void) {
  PIC_sendEOI(15);
}


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
	uint32_t new_table_frame = first_frame() * 0x1000;

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
