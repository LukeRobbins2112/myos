#include <kernel/keyboard.h>
#include <string.h>
#include <stdio.h>
#include <common/inline_assembly.h>

// ------------------------------
// Scancode Lookup Table
// ------------------------------
unsigned char scancode_tables[][256u] =
{
{
  /*0x00*/  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, '\t',  '`',    0,
  /*0x10*/  0,    0,    0,    0,    0,  'q',  '1',    0,    0,    0,  'z',  's',  'a',  'w',  '2',    0,
  /*0x20*/  0,  'c',  'x',  'd',  'e',  '4',  '3',    0,    0,  ' ',  'v',  'f',  't',  'r',  '5',    0,
  /*0x30*/  0,  'n',  'b',  'h',  'g',  'y',  '6',    0,    0,    0,  'm',  'j',  'u',  '7',  '8',    0,
  /*0x40*/  0,  ',',  'k',  'i',  'o',  '0',  '9',    0,    0,  '.',  '/',  'l',  ';',  'p',  '-',    0,
  /*0x50*/  0,    0, '\'',    0,  '[',  '=',    0,    0,    0,    0, '\n',  ']',    0,    0,    0,    0,
  /*0x60*/  0,    0,    0,    0,    0,    0, '\b',    0,    0,  '1',    0,  '4',  '7',    0,    0,    0,
  /*0x70*/'0',  '.',  '2',  '5',  '6',  '8',    0,    0,    0,  '+',  '3',  '-',  '*',  '9',    0,    0,
  /*0x80*/  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
},
{
  /*0x00*/  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, '\t',  '~',    0,
  /*0x10*/  0,    0,    0,    0,    0,  'Q',  '!',    0,    0,    0,  'Z',  'S',  'A',  'W',  '@',    0,
  /*0x20*/  0,  'C',  'X',  'D',  'E',  '$',  '#',    0,    0,  ' ',  'V',  'F',  'T',  'R',  '%',    0,
  /*0x30*/  0,  'N',  'B',  'H',  'G',  'Y',  '^',    0,    0,    0,  'M',  'J',  'U',  '&',  '*',    0,
  /*0x40*/  0,  '<',  'K',  'I',  'O',  ')',  '(',    0,    0,  '>',  '?',  'L',  ':',  'P',  '_',    0,
  /*0x50*/  0,    0,  '"',    0,  '{',  '+',    0,    0,    0,    0, '\n',  '}',    0,    0,    0,    0,
  /*0x60*/  0,    0,    0,    0,    0,    0, '\b',    0,    0,  '1',    0,  '4',  '7',    0,    0,    0,
  /*0x70*/'0',  '.',  '2',  '5',  '6',  '8',    0,    0,    0,  '+',  '3',  '-',  '*',  '9',    0,    0,
  /*0x80*/  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
}
};

// --------------------------------------
// Keyboard State and Other Shared Context
// --------------------------------------

static keyboard_state_t Keyboard_State = {
  .break_code = 0,
  .shift_pressed = 0,
  .alt_pressed = 0,
  .ctrl_pressed = 0,
  .caps_lock = 0  
};

static key_input_t  key_buffer[RING_BUFFER_SIZE];
static key_input_t* buf_head;
static key_input_t* buf_tail;
static uint32_t       key_input_count;


// **************************************************************************************
// **************************************************************************************
// Main Keyboard Implementation
// **************************************************************************************
// **************************************************************************************

void initialize_keyboard_state(){
  buf_head = &key_buffer[0];
  buf_tail = buf_head;
  key_input_count = 0;
}

void push_key_event(key_input_t key_input){
  key_input_count++;
  if (buf_head == &key_buffer[RING_BUFFER_SIZE-1]){
    buf_head = &key_buffer[0]; // Wrap around
  }

  // Copy new input structure, advance pointer
  memcpy(buf_head, &key_input, sizeof(key_input_t));
  buf_head++;
  // printf("key_input_count: %d\n", key_input_count);
}

uint8_t pop_key_event(key_input_t* dest){
  if (key_input_count == 0){
    return 0; // Nothing to read
  }

  if (buf_tail == &key_buffer[RING_BUFFER_SIZE-1]){
    buf_tail = &key_buffer[0]; // Wrap around
  }

  // Update count
  key_input_count--;

  // Populate destination
  memcpy(dest, buf_tail, sizeof(key_input_t));

  // Advance pointer
  buf_tail++;

  // We successfully got a key input
  return 1;
}


void process_scan_code(uint8_t scan_code){

  switch(scan_code){

  case RELEASE_PREFIX:
    {
      Keyboard_State.break_code = 1;
      return;
    }
    
  case LEFT_SHIFT:
  case RIGHT_SHIFT:
    {
      Keyboard_State.shift_pressed = (Keyboard_State.break_code) ? 0 : 1;
      Keyboard_State.break_code = 0;
      break;
    }
    
  case ALT:
    {
      Keyboard_State.alt_pressed = (Keyboard_State.break_code) ? 0 : 1;
      Keyboard_State.break_code = 0;
      break;
    }

  case CONTROL:
    {
      Keyboard_State.ctrl_pressed = (Keyboard_State.break_code) ? 0 : 1;
      Keyboard_State.break_code = 0;
      break;
    }

    // @TODO implement numlock, scroll lock
  case CAPSLOCK:
    {
      // Only turn on/off on keypress, not release
      if (!Keyboard_State.break_code){
	Keyboard_State.caps_lock = !Keyboard_State.caps_lock;
      }
      Keyboard_State.break_code = 0;
      break;
    }
  
  default:
    {
      key_input_t newKeyInput;
      uint8_t table = (Keyboard_State.shift_pressed || Keyboard_State.caps_lock) ? 1 : 0;
      newKeyInput.ascii_value = scancode_tables[table][scan_code];
      newKeyInput.key_code = scan_code;
      newKeyInput.scroll_lock = 0; // @TODO implement this
      newKeyInput.num_lock = 0; // @TODO implement this
      newKeyInput.caps_lock = Keyboard_State.caps_lock;
      newKeyInput.left_shift = Keyboard_State.shift_pressed;
      newKeyInput.right_shift = Keyboard_State.shift_pressed;
      newKeyInput.left_alt = Keyboard_State.alt_pressed;
      newKeyInput.right_alt = Keyboard_State.alt_pressed;
      newKeyInput.left_ctrl = Keyboard_State.ctrl_pressed;
      newKeyInput.right_ctrl = Keyboard_State.ctrl_pressed;
      newKeyInput.rep_if_set = 0; // @TODO implement this
      newKeyInput.rel_if_set = (Keyboard_State.break_code) ? RELEASED : PRESSED; 
      newKeyInput.unicode_value = 0; // @TODO implement this

      push_key_event(newKeyInput);
      Keyboard_State.break_code = 0;
    }

    // If it wasn't a break_code flag, clear that
    if (scan_code != RELEASE_PREFIX){
      
    }
  
  
  }

}

