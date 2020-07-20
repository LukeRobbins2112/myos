// IRQ C handlers, called from irq_handlers_as

#include "common/inline_assembly.h"
#include "kernel/PIC.h"
#include "kernel/pmm.h"
#include "stdio.h"


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
  uint8_t input = inb(0x60);
  PIC_sendEOI(1);
  printf("Input: %x\n", input);
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
