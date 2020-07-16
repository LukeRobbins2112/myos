#include "kernel/ps2controller.h"
#include "common/inline_assembly.h"
#include "stdio.h"


uint8_t read_data_port(){
  //WAIT_FOR_READ();
  uint8_t result = inb(PS2_DATA_PORT);
  return result;
}

void write_data_port(uint8_t val){
  WAIT_FOR_WRITE();
  outb(PS2_DATA_PORT, val);
}

uint8_t read_status_register(){
  WAIT_FOR_READ();
  uint8_t result = inb(PS2_STAT_REG);
  return result;
}

void write_command_register(uint8_t val){
  WAIT_FOR_WRITE();
  outb(PS2_CMD_REG, val);
}

void flush_output_buffer(){
  // Do a couple of dummy data reads
  for (int i = 0; i < 3; i++){
    read_data_port();
  }
}


void initialize_ps2_controller(){

  // Disable devices
  flush_output_buffer();
  write_command_register(DISABLE_PORT_1);
  write_command_register(DISABLE_PORT_2);
  flush_output_buffer();

  // Request initial controller configuration byte
  write_command_register(READ_CONFIG_BYTE);
  WAIT_FOR_READ();
  uint8_t configByte = read_data_port();
  uint8_t dualChannel = (configByte & (0x1 << 5));
  
  printf("Original Controller Config byte: %d\n", configByte);
  printf("Dual Channel: %s\n", dualChannel ? "yes" : "no");

  // Set configuration byte
  configByte &= ~(1 << 0);
  configByte &= ~(1 << 1);
  configByte &= ~(1 << 6);
  printf("New Controller Config Byte: %d\n", configByte);
  write_command_register(WRITE_CONFIG_BYTE);
  write_data_port(configByte);

  // Perform controller self test
  write_command_register(TEST_PS2_CONT);
  WAIT_FOR_READ();
  uint8_t status = read_data_port();
  printf("Controller self test result: %d\n", status);
  printf("Controller test %s\n", (status == 0x55) ? "Passed" : "Failed");

  // @TODO:
  // 7: Determine if there are 2 channels
  // 8: Perform interface tests
  // 9: Enable devices
  // 10: Reset devices
  
}
    
