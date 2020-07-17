#ifndef _PS2_CONTROLLER_H
#define _PS2_CONTROLLER_H

#include <stdint.h>
#include <common/inline_assembly.h>

// ---------------------
// Constants
// ---------------------

#define PS2_DATA_PORT 0x60
#define PS2_STAT_REG  0x64  // Read
#define PS2_CMD_REG   0x64  // Write

// Commands
#define READ_CONFIG_BYTE   0x20
#define WRITE_CONFIG_BYTE  0x60

#define DISABLE_PORT_2     0xA7
#define ENABLE_PORT_2      0xA8
#define TEST_PORT_2        0xA9
#define WRITE_PORT_2       0xD4

#define TEST_PORT_1        0xAB
#define DISABLE_PORT_1     0xAD
#define ENABLE_PORT_1      0xAE

#define DIAG_DUMP          0xAC
#define TEST_PS2_CONT      0xAA
#define IDENTIFY_DEVICE    0xF2
#define ENABLE_SCANNING    0xF4
#define DISABLE_SCANNING   0xF5
#define RESET_DEVICE       0xFF // written to data port

#define PORT_1_DEVICE 0x1
#define PORT_2_DEVICE 0x2

// ----------------------
// Helper macros
// ---------------------
// Output buf status (bit 0) must be set before reading
// Input buf status (bit 1) must be clear before writing
#define WAIT_FOR_READ()   while ((inb(PS2_CMD_REG) & 0x1) == 0x0) io_wait();
#define WAIT_FOR_WRITE()  while ((inb(PS2_CMD_REG) & 0x2) != 0x0) io_wait();

// ---------------------------
// Controller Functions
// ---------------------------

uint8_t read_data_port();
void initialize_ps2_controller();

#endif // _PS2_CONTROLLER_H
