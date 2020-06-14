#include <stdint.h>
#include "kernel/idt.h"
#include "kernel/PIC.h"

// IDT constants
#define KERNEL_CODE_SEGMENT 0x8
#define INTERRUPT_GATE 0x8E


void idt_init(){

  extern int load_idt();

  // Refer to assembly ISRs
  extern int irq0();
  extern int irq1();
  extern int irq2();
  extern int irq3();
  extern int irq4();
  extern int irq5();
  extern int irq6();
  extern int irq7();
  extern int irq8();
  extern int irq9();
  extern int irq10();
  extern int irq11();
  extern int irq12();
  extern int irq13();
  extern int irq14();
  extern int irq15();

  // Address of ISRs
  uint32_t irq0_addr;
  uint32_t irq1_addr;
  uint32_t irq2_addr;
  uint32_t irq3_addr;
  uint32_t irq4_addr;
  uint32_t irq5_addr;
  uint32_t irq6_addr;
  uint32_t irq7_addr;
  uint32_t irq8_addr;
  uint32_t irq9_addr;
  uint32_t irq10_addr;
  uint32_t irq11_addr;
  uint32_t irq12_addr;
  uint32_t irq13_addr;
  uint32_t irq14_addr;
  uint32_t irq15_addr;

  // Remap PIC
  remap_PIC(MASTER_OFFSET, SLAVE_OFFSET);

  // Fill IDT
  // 256 is the standard size
  struct IDT_entry IDT_entries[256];


  irq0_addr = (uint32_t)irq0;
  IDT_entries[32].offset_low = irq0_addr & 0xFFFF; // Lower 2 bytes
  IDT_entries[32].offset_high = (irq0_addr >> 16) & 0xFFFF; // Higher 2 bytes
  IDT_entries[32].selector = KERNEL_CODE_SEGMENT;
  IDT_entries[32].zero = 0;
  IDT_entries[32].type_attr = INTERRUPT_GATE;

  irq1_addr = (uint32_t)irq1;
  IDT_entries[33].offset_low = irq1_addr & 0xFFFF; // Lower 2 bytes
  IDT_entries[33].offset_high = (irq1_addr >> 16) & 0xFFFF; // Higher 2 bytes
  IDT_entries[33].selector = KERNEL_CODE_SEGMENT;
  IDT_entries[33].zero = 0;
  IDT_entries[33].type_attr = INTERRUPT_GATE;

  irq2_addr = (uint32_t)irq2;
  IDT_entries[34].offset_low = irq2_addr & 0xFFFF; // Lower 2 bytes
  IDT_entries[34].offset_high = (irq2_addr >> 16) & 0xFFFF; // Higher 2 bytes
  IDT_entries[34].selector = KERNEL_CODE_SEGMENT;
  IDT_entries[34].zero = 0;
  IDT_entries[34].type_attr = INTERRUPT_GATE;

  irq3_addr = (uint32_t)irq3;
  IDT_entries[35].offset_low = irq3_addr & 0xFFFF; // Lower 2 bytes
  IDT_entries[35].offset_high = (irq3_addr >> 16) & 0xFFFF; // Higher 2 bytes
  IDT_entries[35].selector = KERNEL_CODE_SEGMENT;
  IDT_entries[35].zero = 0;
  IDT_entries[35].type_attr = INTERRUPT_GATE;

  irq4_addr = (uint32_t)irq4;
  IDT_entries[36].offset_low = irq4_addr & 0xFFFF; // Lower 2 bytes
  IDT_entries[36].offset_high = (irq4_addr >> 16) & 0xFFFF; // Higher 2 bytes
  IDT_entries[36].selector = KERNEL_CODE_SEGMENT;
  IDT_entries[36].zero = 0;
  IDT_entries[36].type_attr = INTERRUPT_GATE;

  irq5_addr = (uint32_t)irq5;
  IDT_entries[37].offset_low = irq5_addr & 0xFFFF; // Lower 2 bytes
  IDT_entries[37].offset_high = (irq5_addr >> 16) & 0xFFFF; // Higher 2 bytes
  IDT_entries[37].selector = KERNEL_CODE_SEGMENT;
  IDT_entries[37].zero = 0;
  IDT_entries[37].type_attr = INTERRUPT_GATE;

  irq6_addr = (uint32_t)irq6;
  IDT_entries[38].offset_low = irq6_addr & 0xFFFF; // Lower 2 bytes
  IDT_entries[38].offset_high = (irq6_addr >> 16) & 0xFFFF; // Higher 2 bytes
  IDT_entries[38].selector = KERNEL_CODE_SEGMENT;
  IDT_entries[38].zero = 0;
  IDT_entries[38].type_attr = INTERRUPT_GATE;

  irq7_addr = (uint32_t)irq7;
  IDT_entries[39].offset_low = irq7_addr & 0xFFFF; // Lower 2 bytes
  IDT_entries[39].offset_high = (irq7_addr >> 16) & 0xFFFF; // Higher 2 bytes
  IDT_entries[39].selector = KERNEL_CODE_SEGMENT;
  IDT_entries[39].zero = 0;
  IDT_entries[39].type_attr = INTERRUPT_GATE;

  irq8_addr = (uint32_t)irq8;
  IDT_entries[40].offset_low = irq8_addr & 0xFFFF; // Lower 2 bytes
  IDT_entries[40].offset_high = (irq8_addr >> 16) & 0xFFFF; // Higher 2 bytes
  IDT_entries[40].selector = KERNEL_CODE_SEGMENT;
  IDT_entries[40].zero = 0;
  IDT_entries[40].type_attr = INTERRUPT_GATE;

  irq9_addr = (uint32_t)irq9;
  IDT_entries[41].offset_low = irq9_addr & 0xFFFF; // Lower 2 bytes
  IDT_entries[41].offset_high = (irq9_addr >> 16) & 0xFFFF; // Higher 2 bytes
  IDT_entries[41].selector = KERNEL_CODE_SEGMENT;
  IDT_entries[41].zero = 0;
  IDT_entries[41].type_attr = INTERRUPT_GATE;

  irq10_addr = (uint32_t)irq10;
  IDT_entries[42].offset_low = irq10_addr & 0xFFFF; // Lower 2 bytes
  IDT_entries[42].offset_high = (irq10_addr >> 16) & 0xFFFF; // Higher 2 bytes
  IDT_entries[42].selector = KERNEL_CODE_SEGMENT;
  IDT_entries[42].zero = 0;
  IDT_entries[42].type_attr = INTERRUPT_GATE;

  irq11_addr = (uint32_t)irq11;
  IDT_entries[43].offset_low = irq11_addr & 0xFFFF; // Lower 2 bytes
  IDT_entries[43].offset_high = (irq11_addr >> 16) & 0xFFFF; // Higher 2 bytes
  IDT_entries[43].selector = KERNEL_CODE_SEGMENT;
  IDT_entries[43].zero = 0;
  IDT_entries[43].type_attr = INTERRUPT_GATE;

  irq12_addr = (uint32_t)irq12;
  IDT_entries[44].offset_low = irq12_addr & 0xFFFF; // Lower 2 bytes
  IDT_entries[44].offset_high = (irq12_addr >> 16) & 0xFFFF; // Higher 2 bytes
  IDT_entries[44].selector = KERNEL_CODE_SEGMENT;
  IDT_entries[44].zero = 0;
  IDT_entries[44].type_attr = INTERRUPT_GATE;

  irq13_addr = (uint32_t)irq13;
  IDT_entries[45].offset_low = irq13_addr & 0xFFFF; // Lower 2 bytes
  IDT_entries[45].offset_high = (irq13_addr >> 16) & 0xFFFF; // Higher 2 bytes
  IDT_entries[45].selector = KERNEL_CODE_SEGMENT;
  IDT_entries[45].zero = 0;
  IDT_entries[45].type_attr = INTERRUPT_GATE;

  irq14_addr = (uint32_t)irq14;
  IDT_entries[46].offset_low = irq14_addr & 0xFFFF; // Lower 2 bytes
  IDT_entries[46].offset_high = (irq14_addr >> 16) & 0xFFFF; // Higher 2 bytes
  IDT_entries[46].selector = KERNEL_CODE_SEGMENT;
  IDT_entries[46].zero = 0;
  IDT_entries[46].type_attr = INTERRUPT_GATE;

  irq15_addr = (uint32_t)irq15;
  IDT_entries[47].offset_low = irq15_addr & 0xFFFF; // Lower 2 bytes
  IDT_entries[47].offset_high = (irq15_addr >> 16) & 0xFFFF; // Higher 2 bytes
  IDT_entries[47].selector = KERNEL_CODE_SEGMENT;
  IDT_entries[47].zero = 0;
  IDT_entries[47].type_attr = INTERRUPT_GATE;

  // Load the IDT Table
  struct IDT_ptr idt_ptr;
  idt_ptr.limit = (sizeof(struct IDT_entry) * 256) - 1;
  idt_ptr.base = (uint32_t)&IDT_entries[0];
  load_idt((uint32_t)&idt_ptr);
}
