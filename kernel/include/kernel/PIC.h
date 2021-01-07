#ifndef _PIC_H
#define _PIC_H

#include "common/inline_assembly.h"

// --------------------------------
// PIC constants
// --------------------------------

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

#define MASTER_OFFSET   0x20            /* Re-mapped IRQs */
#define SLAVE_OFFSET    0x28            /* Re-mapped IRQs */

#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW3_SLAVE_LOC  0x04            /* Master IRQ2 has slave */
#define ICW3_SLAVE_ATT  0x02            /* Master IRQ to which slave attaches*/
 
#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

// OCW3 Commands
#define PIC_READ_IRR 0xA
#define PIC_READ_ISR 0xB

// --------------------------------
// PIC functions
// --------------------------------
void remap_PIC(uint8_t master_offset, uint8_t slave_offset);



#endif // _PIC_H


