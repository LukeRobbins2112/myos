// IRQ C handlers, called from irq_handlers_as

#include "common/inline_assembly.h"
#include "kernel/PIC.h"
#include "kernel/pmm.h"
#include "stdio.h"
#include "kernel/keyboard.h"
#include "kernel/timer.h"
#include "kernel/sleep.h"
#include "kernel/multitasking.h"

// --------------------------------------------------
// Constants
// --------------------------------------------------
#define PIC_EOI		0x20		/* End-of-interrupt command code */

// --------------------------------------------------

// --------------------------------------------------
// Data
// --------------------------------------------------
uint64_t time_slice_remaining = TIME_SLICE_LENGTH_MS;

// --------------------------------------------------


static void PIC_sendEOI(unsigned char irq)
{
  if(irq >= 8)
    outb(PIC2_COMMAND, PIC_EOI);
  
  outb(PIC1_COMMAND, PIC_EOI);
}


// Individual Interrupt Service ROutines
void irq0_handler(void) {
  // Increment  timer
  clock_tick();

  // @TODO double-wrapping wake_sleeping_tasks - might want to...
  // ... only have these locks in called functions
  // Lock scheduler locks
  lock_stuff();

  // Now perform handler code / timer updates
  wake_sleeping_tasks();

  // Handle end-of-time-slice
  // For safety, make sure we didn't wrap around the unsigned int
  if (time_slice_remaining == 0 || time_slice_remaining > TIME_SLICE_LENGTH_MS){
    printf("Task switch - time = %d\n", time_slice_remaining);

    time_slice_remaining = TIME_SLICE_LENGTH_MS;
    //switch_to_next_task();
    schedule();
  } else {
    time_slice_remaining -= ms_per_tick();
  }
  
  // Unlock scheduler
  unlock_stuff();
    
  // Send EOI, we got what we needed from the interrupt
  PIC_sendEOI(0);

}
 
void irq1_handler(void) {
  //breakpoint("irq1");
  uint8_t input = inb(0x60);
  PIC_sendEOI(1);
  process_scan_code(input);
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
