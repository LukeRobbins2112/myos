#ifndef _VMM_H
#define _VMM_H

#include "kernel/paging.h"

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


#endif
