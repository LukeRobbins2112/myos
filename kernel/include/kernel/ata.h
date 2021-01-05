#ifndef _ATA_H
#define _ATA_H

#include <stdint.h>

// ----------------------------------------
// Controller I/O Port Registers
// ----------------------------------------

#define PRIMARY_BUS_PORT_BASE       0x1F0
#define SECONDARY_BUS_PORT_BASE     0x170
#define PRIMARY_BUS_CTRL_BASE       0x3F6
#define SECONDARY_BUS_CTRL_BASE     0x376
#define PRIMARY_BUS_ALT_STATUS      0x3F6
#define SECONDARY_BUS_ALT_STATUS    0x376

#define MASTER_DRIVE 0xA0
#define SLAVE_DRIVE  0xB0

// IO Port register offsets
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

// Control register offfsets 
#define ALT_STATUS_REG     0x0
#define DEV_CTRL_REG       0x1
#define DRIVE_ADDR_REG     0x1

// ----------------------------------------
// Command Port Commands
// ----------------------------------------

#define READ_SECTORS_EXT  0x24
#define WRITE_SECTORS_EXT 0x34
#define FLUSH_CACHE 0xE7
#define IDENTIFY          0xEC

// ----------------------------------------
// Control Port Commands
// ----------------------------------------

#define SRST_CLEAR 0x0
#define nIEN       0x2
#define SRST       0x4

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
// Read / Write Constants
// ----------------------------------------

#define LBA_MODE  0x40

// Polling mode, either once or immediate
#define ATA_ONCE  0x0
#define ATA_WAIT  0x1

extern uint8_t CURRENT_DRIVE;

// ----------------------------------------
// Driver implementation
// ----------------------------------------

void detect_and_init();
void reset_controller();
uint8_t read_status(uint8_t drive);
uint8_t select_drive(uint8_t drive);
uint8_t ata_wait(uint8_t status, uint8_t mode);
void check_PIO_status();
void write_pio(uint16_t sector_count, uint32_t LBA_low4, uint16_t LBA_high2);
void read_pio(uint16_t sector_count, uint32_t LBA_low4, uint16_t LBA_high2);
void read_sectors();
void write_sectors();


#endif
