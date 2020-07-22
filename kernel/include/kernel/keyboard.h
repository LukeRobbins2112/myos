#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include <stdint.h>

// --------------------------------------
// Define useful constants/codes
// --------------------------------------

#define RING_BUFFER_SIZE 64  // Storage of incoming KeyInput structs

// Used for key code translation
#define RELEASE_PREFIX  0xF0
#define EXTRAS_PREFIX   0xE0

#define ALT         0x11
#define LEFT_SHIFT  0x12
#define RIGHT_SHIFT 0x59
#define CONTROL     0x14 // @TODO separately list left and right?
#define CAPSLOCK    0x58
#define NUMLOCK     0x77
#define SCROLL_LOCK 0x7E

enum KeyState{
 RELEASED = 0,
 PRESSED = 1
};

// --------------------------------------
// Input management Structures
// --------------------------------------

typedef struct KeyInput {
  char ascii_value;
  char key_code;
  char scroll_lock : 1;
  char num_lock    : 1;
  char caps_lock   : 1;
  char reserved    : 5;
  char left_shift  : 1;
  char right_shift : 1;
  char left_alt    : 1;
  char right_alt   : 1;
  char left_ctrl   : 1;
  char right_ctrl  : 1;
  char rep_if_set  : 1;
  char rel_if_set  : 1;
  short unicode_value;
} key_input_t;

typedef struct Keyboardstate {
  char break_code;
  char shift_pressed;
  char alt_pressed;
  char ctrl_pressed;
  char caps_lock;
} keyboard_state_t;

// -------------------
// Keycode lookup
// -------------------
extern unsigned char scancode_tables[][256u];

// -------------------
// Public Interface
// -------------------
void process_scan_code(uint8_t scan_code);
uint8_t pop_key_event(key_input_t* dest);
void initialize_keyboard_state();
#endif
