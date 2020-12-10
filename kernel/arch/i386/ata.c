#include <kernel/ata.h>
#include <common/inline_assembly.h>
#include <stdio.h>



void detect_and_init(){

  CLI();

  // Check for floating bus (no drive)
  uint8_t initial_status = inb(PRIMARY_BUS_PORT_BASE + STATUS_REG_OFF);
  if (initial_status == INVALID_ATA_STATUS){
    printf("Invalid ATA status\n");
    STI();
    return;
  }

  // Detect controller I/O ports
  // Write different data to R/W ports, then try to read back again
  uint8_t in_val0 = 0xF2;
  uint8_t in_val1 = 0xF3;
  uint8_t in_val2 = 0xF4;
  outb(PRIMARY_BUS_PORT_BASE + SECT_COUNT_REG_OFF, in_val0);
  outb(PRIMARY_BUS_PORT_BASE + LBA_LO_REG_OFF, in_val1);
  outb(PRIMARY_BUS_PORT_BASE + LBA_MID_REG_OFF, in_val2);

  uint8_t out_val0 = inb(PRIMARY_BUS_PORT_BASE + SECT_COUNT_REG_OFF);
  uint8_t out_val1 = inb(PRIMARY_BUS_PORT_BASE + LBA_LO_REG_OFF);
  uint8_t out_val2 = inb(PRIMARY_BUS_PORT_BASE + LBA_MID_REG_OFF);

  if ((in_val0 != out_val0) || (in_val1 != out_val1) || (in_val2 != out_val2)){
    printf("Values written to ports do not match results read\n");
    printf("%x -- %x, %x -- %x, %x -- %x\n", in_val0, out_val0, in_val1, out_val1, in_val2, out_val2);
    STI();
    return;
  } else {
    printf("%x -- %x, %x -- %x, %x -- %x\n", in_val0, out_val0, in_val1, out_val1, in_val2, out_val2);
  }
  

  // IDENTIFY

  // Select master drive
  outb(PRIMARY_BUS_PORT_BASE + DRIVE_HEAD_REG_OFF, MASTER_DRIVE);

  // Zero-out R/W registerss
  outb(PRIMARY_BUS_PORT_BASE + SECT_COUNT_REG_OFF, 0x0);
  outb(PRIMARY_BUS_PORT_BASE + LBA_LO_REG_OFF, 0x0);
  outb(PRIMARY_BUS_PORT_BASE + LBA_MID_REG_OFF, 0x0);
  outb(PRIMARY_BUS_PORT_BASE + LBA_HI_REG_OFF, 0x0);
  
  //Send IDENTIFY command, then read status port
  outb(PRIMARY_BUS_PORT_BASE + CMD_REG_OFF, IDENTIFY);
  uint8_t status = inb(PRIMARY_BUS_PORT_BASE + STATUS_REG_OFF);

  if (status == 0){
    printf("Drive does not exist; status = %d\n", status);
    STI();
    return;
  } else {
    printf("Drive exists\n");
  }
    


  // At this point we identified a drive
  // Poll while status is BSY
  uint32_t poll_count = 0;
  while(inb(PRIMARY_BUS_PORT_BASE + STATUS_REG_OFF) & BSY){
    poll_count++;
    if (poll_count % 1000 == 0){
      printf("poll_count: %d\n", poll_count);
    }
  }

  printf("final poll_count: %d\n", poll_count);

  // Check to make sure drive is ATA
  uint8_t lba_mid = inb(PRIMARY_BUS_PORT_BASE + LBA_MID_REG_OFF);
  uint8_t lba_hi = inb(PRIMARY_BUS_PORT_BASE + LBA_HI_REG_OFF);
  if (lba_mid || lba_hi){
    printf("lba_mid or lba_hi is nonzero -- non-ATA drive\n");
    printf("lba_mid = %d, lba_hi = %d\n", lba_mid, lba_hi);
    STI();
    return;
  }

  // Continue polling until DRW is set (or ERR)
  uint8_t mask = DRQ | ERR;
  poll_count = 0;
  while (!(status = inb(PRIMARY_BUS_PORT_BASE + STATUS_REG_OFF) & mask)){
    poll_count++;
    if (poll_count % 1000 == 0){
      printf("second poll_count: %d\n", poll_count);
    }
  }

  printf("second final poll_count: %d\n", poll_count);

  if (status & ERR){
    printf("Error received when performing second poll. Status = %x\n", status);
  }
  if (status & DRQ){
    printf("Status indicates DRQ -- PIO data to transfer\n");
  }


  uint16_t identify_data[256];

  for (int i = 0; i < 256; i++){
    identify_data[i] = inw(PRIMARY_BUS_PORT_BASE + DATA_REG_OFF);
  }

  printf("Read identify data\n");
  STI();
}
