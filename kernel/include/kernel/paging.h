#ifndef _PAGING_H
#define _PAGING_H

#include <stdint.h>

// --------------------------------------------
// Structure Definitions
// --------------------------------------------

typedef struct page {
  uint32_t present    : 1;
  uint32_t rw         : 1;
  uint32_t user       : 1;
  uint32_t write_thru : 1;
  uint32_t cached     : 1;
  uint32_t accessed   : 1;
  uint32_t dirty      : 1;
  uint32_t PAT        : 1;
  uint32_t global     : 1;
  uint32_t avail      : 3;
  uint32_t frame      : 20;

}__attribute__((packed)) page_t;


typedef struct page_table {
  page_t pages[1024];
} page_table_t;

typedef struct page_directory {

  // Our actual page_tables lookup structure
  page_table_t* page_tables[1024];

  // Physical locations of page tables
  uint32_t page_tables_physical[1024];

  // Address of page_tables_physical
  uint32_t physicalTablesAddr;

} page_directory_t;


// --------------------------------------------
// Paging Functions
// --------------------------------------------

// Set up environment, page directories. Enable paging
void initialize_paging();

// Load new page table directory into cr3
void switch_page_directory(page_directory_t* newDir);

// Retrieve pointer to the required page
// (create == 1): If the relevant page table doesn't exist, create it
page_t* get_page(uint32_t address, int create, page_directory_t* dir);

// Handler for page faults
// void page_fault(registers_t regs);


#endif
