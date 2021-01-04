#include <kernel/ata.h>
#include <common/inline_assembly.h>
#include <stdio.h>

// Flag to track current drive selection
uint8_t CURRENT_DRIVE = MASTER_DRIVE;


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
    printf("%x ", (uint32_t)identify_data[i]);
    if (i && i % 8 == 0){
      printf("\n");
    }
  }

  printf("Read identify data\n");
  STI();
}

// Returns status (1 = success, 0 = failure)
uint8_t select_drive(uint8_t drive){
  // Already selected, nothing to do
  if (drive == CURRENT_DRIVE){
    return 0;
  }

  uint8_t status;

  // No matter which drive is requested, we first must check the
  // status of the currently selected drive to make sure that it
  // is not actively modifying status
  status = inb(PRIMARY_BUS_PORT_BASE + STATUS_REG_OFF);
  uint8_t mask = BSY | DRQ | ERR;
  if (status & mask){
    printf("Not ready to select drive\n");
  }

  
  // Select drive, then delay 400ns
  outb(PRIMARY_BUS_PORT_BASE + DRIVE_HEAD_REG_OFF, drive);
  for (int i = 0; i < 4; i++){
    inb(PRIMARY_BUS_ALT_STATUS);
  }

  // Read the Status register to clear pending interrupts
  // Ignore value
  inb(PRIMARY_BUS_PORT_BASE + STATUS_REG_OFF);

  // Read Status register one more time, use this to determine status
  status = inb(PRIMARY_BUS_PORT_BASE + STATUS_REG_OFF);

  if (status & mask){
    printf("Drive select failed\n");
    return 0;
  }
  
  CURRENT_DRIVE = drive;
  return 1;
}


uint8_t read_status(uint8_t drive){

  uint8_t status;

  // No matter which drive is requested, we first must check the
  // status of the currently selected drive to make sure that it
  // is not actively modifying status
  status = inb(PRIMARY_BUS_PORT_BASE + STATUS_REG_OFF);
  uint8_t mask = BSY | DRQ | ERR;
  uint8_t pollcount = 0;
  while (status & mask){
    status = inb(PRIMARY_BUS_PORT_BASE + STATUS_REG_OFF);
    pollcount++;
    if (pollcount % 100 == 0){
      printf("poll count = %d\n", pollcount);
    }
  }

  printf("Drive done, about to select drive; pollcount= %d\n", pollcount);

  // If not current drive, select and wait for change
  if (drive != CURRENT_DRIVE){
    printf("Reading status from non-active drive\n");
    outb(PRIMARY_BUS_PORT_BASE + DRIVE_HEAD_REG_OFF, drive);
    for (int i = 0; i < 4; i++){
      status = inb(PRIMARY_BUS_PORT_BASE + STATUS_REG_OFF);
    }

  }

  // Now get the status value we'll actually use
  status = inb(PRIMARY_BUS_PORT_BASE + STATUS_REG_OFF);
  return status;
}

void write_pio(uint16_t sector_count, uint32_t LBA_low4, uint16_t LBA_high2){
  // Read LBA mode
  // @TODO this assumes master drive
  outb(PRIMARY_BUS_PORT_BASE + DRIVE_HEAD_REG_OFF, LBA_MODE);

  // Sectorcount high byte
  outb(PRIMARY_BUS_PORT_BASE + SECT_COUNT_REG_OFF, (sector_count >> 8) & 0xFF);

  // LBA bytes 4, 5, 6
  outb(PRIMARY_BUS_PORT_BASE + LBA_LO_REG_OFF, (LBA_low4 >> 24) & 0xFF);
  outb(PRIMARY_BUS_PORT_BASE + LBA_MID_REG_OFF, (LBA_high2) & 0xFF);
  outb(PRIMARY_BUS_PORT_BASE + LBA_HI_REG_OFF, (LBA_high2 >> 8) & 0xFF);

  // Sectorcount low byte
  outb(PRIMARY_BUS_PORT_BASE + SECT_COUNT_REG_OFF, (sector_count) & 0xFF);

  // LBA bytes 1, 2, 3
  outb(PRIMARY_BUS_PORT_BASE + LBA_LO_REG_OFF, (LBA_low4) & 0xFF);
  outb(PRIMARY_BUS_PORT_BASE + LBA_MID_REG_OFF, (LBA_low4 >> 8) & 0xFF);
  outb(PRIMARY_BUS_PORT_BASE + LBA_HI_REG_OFF, (LBA_low4 >> 16) & 0xFF);

  // Write command
  outb(PRIMARY_BUS_PORT_BASE + CMD_REG_OFF, WRITE_SECTORS_EXT);
}

void read_pio(uint16_t sector_count, uint32_t LBA_low4, uint16_t LBA_high2){

  // Read LBA mode
  // @TODO this assumes master drive
  outb(PRIMARY_BUS_PORT_BASE + DRIVE_HEAD_REG_OFF, LBA_MODE);

  // Sectorcount high byte
  outb(PRIMARY_BUS_PORT_BASE + SECT_COUNT_REG_OFF, (sector_count >> 8) & 0xFF);

  // LBA bytes 4, 5, 6
  outb(PRIMARY_BUS_PORT_BASE + LBA_LO_REG_OFF, (LBA_low4 >> 24) & 0xFF);
  outb(PRIMARY_BUS_PORT_BASE + LBA_MID_REG_OFF, (LBA_high2) & 0xFF);
  outb(PRIMARY_BUS_PORT_BASE + LBA_HI_REG_OFF, (LBA_high2 >> 8) & 0xFF);

  // Sectorcount low byte
  outb(PRIMARY_BUS_PORT_BASE + SECT_COUNT_REG_OFF, (sector_count) & 0xFF);

  // LBA bytes 1, 2, 3
  outb(PRIMARY_BUS_PORT_BASE + LBA_LO_REG_OFF, (LBA_low4) & 0xFF);
  outb(PRIMARY_BUS_PORT_BASE + LBA_MID_REG_OFF, (LBA_low4 >> 8) & 0xFF);
  outb(PRIMARY_BUS_PORT_BASE + LBA_HI_REG_OFF, (LBA_low4 >> 16) & 0xFF);

  // Read sectors command
  outb(PRIMARY_BUS_PORT_BASE + CMD_REG_OFF, READ_SECTORS_EXT);
}


void read_sectors(){
  uint16_t data[256];
  for (int i = 0; i < 256; i++){
    data[i] = inw(PRIMARY_BUS_PORT_BASE + DATA_REG_OFF);
  }
}

void write_sectors(uint16_t data[256]){
  for (int i = 0; i < 256; i++){
    outw(PRIMARY_BUS_PORT_BASE + DATA_REG_OFF, data[i]);
  }
  
  // Cache flush
  outb(PRIMARY_BUS_PORT_BASE + CMD_REG_OFF, FLUSH_CACHE);
}
