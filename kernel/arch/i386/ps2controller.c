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

void cmd_dev_port(uint8_t command, uint8_t device){
  // Sanity check
  if (device > 0x2){
    printf("Error - unknown device\n");
  }
  
  if (device == PORT_2_DEVICE){
    write_command_register(WRITE_PORT_2);    
  }
  write_data_port(command);
}

uint8_t enable_scanning(uint8_t device){

  cmd_dev_port(ENABLE_SCANNING, device);

  WAIT_FOR_READ();
  uint8_t ack = read_data_port();
  if (ack == 0xFA){
    printf("Enable scan Acked\n");
  } else {
    printf("Enable scan reply: %x\n", ack);
  }
  
  return ack;
}

uint16_t identify_device(uint8_t device){
  // Do some dummy reads
  flush_output_buffer();

  // 11: Identify port 1 device
  if (device == PORT_2_DEVICE){
    write_command_register(WRITE_PORT_2);
  }
  write_data_port(DISABLE_SCANNING);
  WAIT_FOR_READ();
  uint8_t ack = read_data_port();
  if (ack == 0xFA){
    printf("Disable scan Acked\n");
  } else {
    printf("Disable scan reply: %x\n", ack);
  }

  if (device == PORT_2_DEVICE){
    write_command_register(WRITE_PORT_2);
  }
  write_data_port(IDENTIFY_DEVICE);
  WAIT_FOR_READ();
  ack = read_data_port();
  if (ack == 0xFA){

    WAIT_FOR_READ();
    uint16_t deviceID = read_data_port();
    printf("DeviceID: %x\n", deviceID);
    if (deviceID == 0xAB){
      WAIT_FOR_READ();
      uint16_t deviceID1 = read_data_port();
      printf("DeviceID1: %x\n", deviceID1);
      deviceID = ((deviceID << 8) | (deviceID1));
      printf("Dull DeviceID: %x\n", deviceID);
    }

    enable_scanning(device);

    return deviceID;
    
  } else {
    printf("Reply was %x\n", ack);
  }
  
  return 0xFF; // Failure
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
  //printf("Dual Channel: %s\n", dualChannel ? "yes" : "no");

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
  //printf("Controller self test result: %d\n", status);
  printf("Controller test %s\n", (status == 0x55) ? "Passed" : "Failed");

  // @TODO:
  // 7: Determine if there are 2 channels
  write_command_register(ENABLE_PORT_2);
  write_command_register(READ_CONFIG_BYTE);
  WAIT_FOR_READ();
  configByte = read_data_port();
  dualChannel = !(configByte & (0x1 << 5));
  printf("Controller config byte: %x\n", configByte);
  printf("Port 2 is %s\n", (dualChannel)? "enabled" : "disabled");

  if (dualChannel){
    write_command_register(DISABLE_PORT_2);
  }
  
  // 8: Perform interface tests
  // Check port 1
  write_command_register(TEST_PORT_1);
  WAIT_FOR_READ();
  uint8_t port1_res = read_data_port();
  printf("Port 1 Test: %d - %s\n", port1_res, port1_res ? "failed" : "passed");

  // Check port 2
  write_command_register(TEST_PORT_2);
  WAIT_FOR_READ();
  uint8_t port2_res = read_data_port();
  printf("Port 2 Test: %d - %s\n", port2_res, port2_res ? "failed" : "passed");
  
  // 9: Enable devices
  write_command_register(ENABLE_PORT_1);
  write_command_register(ENABLE_PORT_2);

  write_command_register(READ_CONFIG_BYTE);
  WAIT_FOR_READ();
  configByte = read_data_port();
  printf("Config byte w/ devices enabled: %x\n", configByte);

  // Enable IRQs
  configByte |= (1 << 0);
  configByte |= (1 << 1);
  printf("New Controller Config Byte: %d\n", configByte);
  write_command_register(WRITE_CONFIG_BYTE);
  write_data_port(configByte);

  // check that configByte was updated
  write_command_register(READ_CONFIG_BYTE);
  WAIT_FOR_READ();
  configByte = read_data_port();
  printf("Final config byte = %x\n", configByte);
  
  // 10: Reset devices
  write_data_port(RESET_DEVICE);
  WAIT_FOR_READ();
  uint8_t port1_reset_status = read_data_port();
  printf("port1 reset status = %x\n", port1_reset_status);
  WAIT_FOR_READ();
  uint8_t self_test = read_data_port();
  printf("port 1 self test = %x\n", self_test);

  write_command_register(WRITE_PORT_2);
  write_data_port(RESET_DEVICE);
  WAIT_FOR_READ();
  uint8_t port2_reset_status = read_data_port();
  printf("port2 reset status = %x\n", port2_reset_status);
  WAIT_FOR_READ();
  self_test = read_data_port();
  printf("port 2 self test = %x\n", self_test);

  // Do some dummy reads
  flush_output_buffer();

  identify_device(PORT_1_DEVICE);
  identify_device(PORT_2_DEVICE);

  /* // 11: Identify port 1 device */
  /* write_data_port(DISABLE_SCANNING); */
  /* WAIT_FOR_READ(); */
  /* uint8_t ack = read_data_port(); */
  /* if (ack == 0xFA){ */
  /*   //printf("Disable scan Acked\n"); */
  /* } else { */
  /*   printf("Disable scan reply: %x\n", ack); */
  /* } */
  /* write_data_port(IDENTIFY_DEVICE); */
  /* WAIT_FOR_READ(); */
  /* ack = read_data_port(); */
  /* if (ack == 0xFA){ */

  /*   WAIT_FOR_READ(); */
  /*   uint16_t deviceID = read_data_port(); */
  /*   printf("DeviceID: %x\n", deviceID); */
  /*   if (deviceID == 0xAB){ */
  /*     WAIT_FOR_READ(); */
  /*     uint16_t deviceID1 = read_data_port(); */
  /*     printf("deviceID1: %x\n", deviceID1); */
  /*     deviceID = ((deviceID << 8) | (deviceID1)); */
  /*     printf("full DeviceID: %x\n", deviceID); */
  /*   } */
    
  /* } else { */
  /*   printf("Reply was %x\n", ack); */
  /* } */

  /* // Do some dummy reads */
  /* flush_output_buffer(); */
  
  /* // 12: Identify port 2 device */
  /* write_command_register(WRITE_PORT_2); */
  /* write_data_port(DISABLE_SCANNING); */
  /* WAIT_FOR_READ(); */
  /* ack = read_data_port(); */
  /* if (ack == 0xFA){ */
  /*   //printf("Disable scan Acked\n"); */
  /* } else { */
  /*   printf("Disable scan reply: %x\n", ack); */
  /* } */

  /* write_command_register(WRITE_PORT_2); */
  /* write_data_port(IDENTIFY_DEVICE); */
  /* WAIT_FOR_READ(); */
  /* ack = read_data_port(); */
  /* if (ack == 0xFA){ */
  /*   // printf("Identify Acked\n"); */

  /*   WAIT_FOR_READ(); */
  /*   uint16_t deviceID = read_data_port(); */
  /*   printf("DeviceID: %x\n", deviceID); */
  /*   if (deviceID == 0xAB){ */
  /*     WAIT_FOR_READ(); */
  /*     uint16_t deviceID1 = read_data_port(); */
  /*     printf("deviceID1: %x\n", deviceID1); */
  /*     deviceID = ((deviceID << 8) | (deviceID1)); */
  /*     printf("full DeviceID: %x\n", deviceID); */
  /*   } */
    
  /* } else { */
  /*   printf("Reply was %x\n", ack); */
  /* } */


  /* // Re-enable scanning */
  /* cmd_port_1(ENABLE_SCANNING); */
  /* WAIT_FOR_READ(); */
  /* uint8_t ack = read_data_port(); */
  /* if (ack == 0xFA){ */
  /*   printf("Enable scan Acked\n"); */
  /* } else { */
  /*   printf("Enable scan reply: %x\n", ack); */
  /* } */
  
  /* cmd_port_2(ENABLE_SCANNING); */
  /* WAIT_FOR_READ(); */
  /* ack = read_data_port(); */
  /* if (ack == 0xFA){ */
  /*   printf("Enable scan Acked\n"); */
  /* } else { */
  /*   printf("Enable scan reply: %x\n", ack); */
  /* } */


}
    
