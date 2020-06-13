#include "kernel/descriptor_table.h"


// For accessing assembly function
extern void gdt_flush(uint32_t);
// Internal function
static void init_gdt();
static void gdt_set_gate(int32_t idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

// We want 5 entries for our GDT: null, kernel code, kernel data, user code, user data
gdt_entry_t gdt_entries[5];
gdt_ptr_t gdt_ptr;


// Implementation
void init_descriptor_tables(){
  init_gdt();
}

static void init_gdt(){

  // Set up the actual ptr
  // The offset is the linear address of the table itself, which means that paging applies.
  // The size is the size of the table subtracted by 1. This is because the maximum value
  // of size is 65535, while the GDT can be up to 65536 bytes (a maximum of 8192 entries). 
  gdt_ptr.size = (sizeof(gdt_entry_t) * 5) - 1;
  gdt_ptr.offset = (uint32_t)&gdt_entries[0];

  // Add entries to the table
  gdt_set_gate(0, 0x0, 0x0, 0x0, 0x0);             // Null entry
  gdt_set_gate(1, 0x0, 0xFFFFFFFF, 0x9A, 0xCF);  // Kernel Code
  gdt_set_gate(2, 0x0, 0xFFFFFFFF, 0x92, 0xCF);  // Kernel data
  gdt_set_gate(3, 0x0, 0xFFFFFFFF, 0xFA, 0xCF);  // User space code
  gdt_set_gate(4, 0x0, 0xFFFFFFFF, 0xF2, 0xCF);  // User space data

  gdt_flush((uint32_t)&gdt_ptr);
}

static void gdt_set_gate(int32_t idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {

  // First 2 bytes of base, then third byte of base, then fourth
  gdt_entries[idx].base_low    = (base & 0xFFFF);
  gdt_entries[idx].base_middle = (base >> 16) & 0xFF;
  gdt_entries[idx].base_high   = (base >> 24) & 0xFF;

  // Limit low is first 2 bytes
  // Limit high is first 4 bits of third byte
  // Setting lower 4 bits of lim_high_and_gran (the limit high bits)
  gdt_entries[idx].limit_low         = (limit & 0xFFFF);
  gdt_entries[idx].lim_high_and_gran = (limit >> 16) & 0x0F;

  // Set high 4 bits of lim_high_and_gran
  // These are the actual granularity bits
  gdt_entries[idx].lim_high_and_gran |= (gran & 0xF0);

  // Just set access directly
  gdt_entries[idx].access = access;
}







// -----------------------
// Misc Debugging / Testing
// -----------------------

/*
uint64_t entries[5];
void init_gdt_debug(){
  gdt_ptr.size = (sizeof(uint64_t) * 5) - 1;
  gdt_ptr.offset = (uint32_t)&entries;
  entries[0] = 0x0000000000000000;
  entries[1] = 0x00CF9A000000FFFF;
  entries[2] = 0x00CF92000000FFFF;
  entries[3] = 0x00CFFA000000FFFF;
  entries[4] = 0x00CFF2000000FFFF;

  
  printf("GDT Entries\n");
  for (int i = 0; i < 5; i++){
    printf("Entry %d: 0x%.16llX\n", i, entries[i]);
  }
  
}
*/

/*
static void debug_gdt(){
 for (int i = 0; i < 5; i++){
    printf("GDT Entry %d\n", i);
    printf("base_low: %x\n", gdt_entries[i].base_low);
    printf("base_middle: %x\n", gdt_entries[i].base_middle);
    printf("base_high: %x\n", gdt_entries[i].base_high);
    printf("limit_low: %x\n", gdt_entries[i].limit_low);
    printf("lim_high_and_gran: %x\n", gdt_entries[i].lim_high_and_gran);
    printf("access: %x\n", gdt_entries[i].access);
    printf("-------------------\n\n\n");
  }

  printf("Complete GDT Entries\n");
  for (int i = 0; i < 5; i++){
    printf("0x%04x%04x%02x%02x%02x%02x\n",
	   gdt_entries[i].limit_low,
	   gdt_entries[i].base_low,
	   gdt_entries[i].base_middle,
	   gdt_entries[i].access,
	   gdt_entries[i].lim_high_and_gran,
	   gdt_entries[i].base_high);
  }
}
  

int main(){
  init_descriptor_tables();
  return 0;
}
*/