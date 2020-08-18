#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <common/inline_assembly.h>
 
#include <kernel/tty.h>
 
#include "vga.h"
 
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xC00B8000;
 
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

#define HIST_BUF_SIZE 128
static uint16_t hist_buffer[HIST_BUF_SIZE][VGA_WIDTH];
static uint32_t hist_buf_index = 0;

void scroll_down(){
  if (terminal_row < VGA_HEIGHT){
    return;
  }

  if (hist_buf_index == HIST_BUF_SIZE){
    return;
  }

  uint32_t vga_buf_idx = terminal_row*VGA_WIDTH + terminal_column;
  //memcpy(hist_buffer[hist_buf_index++], &terminal_buffer[vga_buf_idx], VGA_WIDTH*sizeof(uint16_t));

  // Move all the lines up one
  for (size_t y = 0; y < VGA_HEIGHT-1; y++) {
    memcpy(&terminal_buffer[y*VGA_WIDTH], &terminal_buffer[(y+1)*VGA_WIDTH], VGA_WIDTH*sizeof(uint16_t));
  }

  // Clear the last line
  for (size_t x = 0; x < VGA_WIDTH; x++) {
    const size_t index = (VGA_HEIGHT-1) * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(' ', terminal_color);
  }

  // Set terminal row to the last row
  terminal_row = VGA_HEIGHT - 1;
}
 
void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = (uint16_t*) 0xC00B8000; // VGA_MEMORY;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
	  for (size_t x = 0; x < VGA_WIDTH; x++) {
	    const size_t index = y * VGA_WIDTH + x;
	    terminal_buffer[index] = vga_entry(' ', terminal_color);
	  }
	}
}
 
void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}
 
void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	/* if (index >= VGA_WIDTH * VGA_HEIGHT){ */
	/*   for (int i = 0; i < VGA_WIDTH; i++){ */
	/*     terminal_buffer[index] = vga_entry('F', color); */
	/*   } */
	/*   return; */
	/* } */
	
	terminal_buffer[index] = vga_entry(c, color);
}
 
void terminal_putchar(char c) {
	unsigned char uc = c;

	// If we're at the last row, increment the terminal to show it
	if (terminal_row == VGA_HEIGHT){
	  scroll_down();
	}

	// For newline, don't print, just move to next row
	if ('\n' == uc){
	  ++terminal_row;
	  terminal_column = 0;
	  return;
	}

	// Print the character
	terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);

	// If we reached the end of the row, go to next line
	if (++terminal_column == VGA_WIDTH) {
	  terminal_column = 0;
	  ++terminal_row;
	}
}
 
void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}
 
void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}
