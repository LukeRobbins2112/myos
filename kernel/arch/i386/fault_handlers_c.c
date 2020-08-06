#include "kernel/pmm.h"
#include "kernel/vmm.h"
#include "common/inline_assembly.h"

// Page Fault Handler
void page_fault_handler(uint32_t faulting_addr, uint32_t error_code){

  if ((error_code & 0x1) == 0){
    page_t* newpage = get_page(faulting_addr, 1);
    if (!newpage->present && !newpage->frame){
      alloc_frame(newpage, 1, 1);
    }
  }
}
