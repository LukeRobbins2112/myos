#ifndef _IDT_H
#define _IDT_H

#include <stdint.h>

// ---------------------------------
// Struct definition
// Initialize IDT_entries table
// ---------------------------------

struct IDT_ptr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

struct IDT_entry {
  uint16_t offset_low;
  uint16_t selector;
  uint8_t zero;
  uint8_t type_attr;
  uint16_t offset_high;
} __attribute__((packed));



// -----------------------------
// Individual IDT entry setup
// -----------------------------
void setup_interrupt_service_routines();
void setup_page_fault_handler();

// ----------------------
// idt_init() definition
// ----------------------
void idt_init();

#endif // _IDT_H

