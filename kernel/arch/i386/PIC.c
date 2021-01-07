#include "kernel/PIC.h"

void remap_PIC(uint8_t master_offset, uint8_t slave_offset){

  // Get current masks
  unsigned char mask_master, mask_slave;
  mask_master = inb(PIC1_DATA);
  mask_slave = inb(PIC2_DATA);

  // Send initialization instruction to prep for remap
  outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // 0x11
  io_wait();
  outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);  // 0x11
  io_wait();

  // Set new offsets - low 3 bits are zero
  // Normally 0x20 for PIC1, 0x28 for PIC2
  outb(PIC1_DATA, master_offset);
  io_wait();
  outb(PIC2_DATA, slave_offset);
  io_wait();

  // Set master/slave mapping
  outb(PIC1_DATA, ICW3_SLAVE_LOC);
  io_wait();
  outb(PIC2_DATA, ICW3_SLAVE_ATT);
  io_wait();

  // Set 8086 mode
  outb(PIC1_DATA, ICW4_8086);
  io_wait();
  outb(PIC2_DATA, ICW4_8086);
  io_wait();

  // Restore masks
  outb(PIC1_DATA, 0xFF);
  io_wait();
  outb(PIC2_DATA, 0xFF);
  io_wait();
}

void IRQ_set_mask(uint8_t IRQline) {
    uint16_t port;
    uint8_t value;
 
    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) | (1 << IRQline);
    outb(port, value);        
}
 
void IRQ_clear_mask(uint8_t IRQline) {
    uint16_t port;
    uint8_t value;
 
    if(IRQline < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        IRQline -= 8;
    }
    value = inb(port) & ~(1 << IRQline);
    outb(port, value);        
}

// Managing the ISR and IRR
// ocw = operational control word
static uint16_t pic_get_irq_reg(int ocw3){
  outb(PIC1_COMMAND, ocw3);
  outb(PIC2_COMMAND, ocw3);
  return (inb(PIC2_COMMAND) << 8 | inb(PIC1_COMMAND));
}

uint16_t pic_get_irr(){
  return pic_get_irq_reg(PIC_READ_IRR);
}

uint16_t pic_get_isr(){
  return pic_get_irq_reg(PIC_READ_ISR);
}
