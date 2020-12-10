#ifndef _ATA_H
#define _ATA_H

// ----------------------------------------
// Controller I/O Port Registers
// ----------------------------------------

#define PRIMARY_BUS_PORT_BASE       0x1F0
#define SECONDARY_BUS_PORT_BASE     0x170

#define MASTER_DRIVE 0xA0
#define SLAVE_DRIVE  0xB0
 
#define DATA_REG_OFF        0x0
#define ERR_REG_OFF         0x1
#define FEATURE_REG_OFF     0x1
#define SECT_COUNT_REG_OFF  0x2
#define LBA_LO_REG_OFF      0x3
#define LBA_MID_REG_OFF     0x4
#define LBA_HI_REG_OFF      0x5
#define DRIVE_HEAD_REG_OFF  0x6
#define STATUS_REG_OFF      0x7
#define CMD_REG_OFF         0x7

// ----------------------------------------
// Controller Port Commands
// ----------------------------------------

#define IDENTIFY 0xEC

// ----------------------------------------
// Status Register Bitflag Codes
// ----------------------------------------

#define ERR  0x1   // Indicates an error occurred. Send new cmd to clear
#define IDX  0x2   // Index. ALways set to zero
#define CORR 0x4   // Corrected data. Always set to zero
#define DRQ  0x8   // Set when drive has PIO dat to transfer, or is ready to accept PIO data
#define SRV  0x10  // Overlapped Mode Service Request
#define DF   0x20  // Drive Fault Error (does not set ERR)
#define RDY  0x40  // Bit is clear when drive is spun down or after err. Set otherwise
#define BSY  0x80  // Indicates drive is preparing to send/receive data (wait for it to clear). 

#define INVALID_ATA_STATUS 0xFF


// ----------------------------------------
// Driver implementation
// ----------------------------------------

void detect_and_init();



#endif
