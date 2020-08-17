#ifndef _PAGING_H
#define _PAGING_H

#include <stdint.h>

// --------------------------------------------
// Constant Definitions
// --------------------------------------------

#define PAGE_SIZE 0x1000

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
} page_directory_t;


#endif // _PAGING_H
