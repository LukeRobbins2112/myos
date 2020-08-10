#include <kernel/PIT_Timer.h>
#include <common/inline_assembly.h>
#include <stdio.h>

static void send_command(uint8_t command){
  outb(COMMAND_REGISTER, command);
}

static void send_data(uint8_t data){
  outb(CHANNEL0_DATA_PORT, data);
}

void initialize_PIT_timer(uint32_t frequency){

  uint32_t divisor = PIT_INPUT_FREQ / frequency;

  // Send command byte
  send_command(PIT_CONFIG);

  // Send Frequency data bytes
  uint8_t low_byte = (uint8_t)(divisor & 0xFF);
  uint8_t high_byte = (uint8_t)((divisor >> 8) && 0xFF);
  send_data(low_byte);
  send_data(high_byte);
}
