#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <common/inline_assembly.h>
 
#include <kernel/tty.h>
 
#include "vga.h"
 
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define LAST_ROW   VGA_HEIGHT - 1
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xC00B8000;
 
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

#define HIST_BUF_SIZE 64
static uint16_t hist_buffer[HIST_BUF_SIZE][VGA_WIDTH];
static uint32_t hist_buf_index = 0;
static uint32_t term_offset = HIST_BUF_SIZE - VGA_HEIGHT;
static uint32_t term_first_line = 0;
static uint32_t term_last_line = VGA_HEIGHT-1;


void jump_to_end(){
  term_offset = HIST_BUF_SIZE - VGA_HEIGHT;  
  // Redraw the terminal buffer
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    memcpy(&terminal_buffer[y*VGA_WIDTH], hist_buffer[(term_offset+y)], VGA_WIDTH*sizeof(uint16_t));
  }
}

void line_feed(){
  // Move all the lines up one
  for (size_t y = 0; y < HIST_BUF_SIZE-1; y++) {
    memcpy(hist_buffer[y], hist_buffer[(y+1)], VGA_WIDTH*sizeof(uint16_t));
  }

  // Clear the last line
  for (size_t x = 0; x < VGA_WIDTH; x++) {
    hist_buffer[HIST_BUF_SIZE-1][x] = vga_entry(' ', terminal_color);
  }
  
  // Redraw the terminal buffer
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    memcpy(&terminal_buffer[y*VGA_WIDTH], hist_buffer[(term_offset+y)], VGA_WIDTH*sizeof(uint16_t));
  }
  
  // Set terminal row to the last row
  terminal_row = LAST_ROW;  
}

void scroll_down(){
  if ((term_offset+VGA_HEIGHT) == HIST_BUF_SIZE){
    return;
  }

  // Redraw the terminal buffer
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    memcpy(&terminal_buffer[y*VGA_WIDTH], hist_buffer[(term_offset+y+1)], VGA_WIDTH*sizeof(uint16_t));
  }
  
  // Increment the terminal offset in the hist_buffer
  if ((term_offset + VGA_HEIGHT) < HIST_BUF_SIZE){
    term_offset++;
  }
}

void scroll_up(){
  if (term_offset == 0){
    return;
  }

  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    memcpy(&terminal_buffer[y*VGA_WIDTH], hist_buffer[(term_offset+y-1)], VGA_WIDTH*sizeof(uint16_t));
  }

  // Update the terminal offset in the hist_buffer
  term_offset--;
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

  // Initialize hist buffer
  for (size_t y = 0; y < HIST_BUF_SIZE; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      hist_buffer[y][x] = vga_entry(' ', terminal_color);
    }
  }
}
 
void terminal_setcolor(uint8_t color) {
  terminal_color = color;
}
 
void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
  const size_t index = y * VGA_WIDTH + x;
  const size_t term_offset_end = HIST_BUF_SIZE - VGA_HEIGHT;

  // Only draw if we're at the end of the terminal, not if we scrolled up
  if (term_offset == term_offset_end) {
    hist_buffer[term_offset+y][x] = vga_entry(c, color);
    terminal_buffer[index] = vga_entry(c, color);
  } else {
    hist_buffer[term_offset_end+y][x] = vga_entry(c, color);
  }
  
}
 
void terminal_putchar(char c) {
  unsigned char uc = c;
  
  // If we're at the last row, increment the terminal to show it
  if (terminal_row == VGA_HEIGHT){

    line_feed();

    if (term_offset == HIST_BUF_SIZE - VGA_HEIGHT){
      // Clear the last line
      for (size_t x = 0; x < VGA_WIDTH; x++) {
	const size_t index = (VGA_HEIGHT-1) * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(' ', terminal_color);
      }
    }
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

void print_last_line(){
  uint16_t* line = hist_buffer[term_offset-1];
  for (int i = 0; i < VGA_WIDTH; i++){
    terminal_putchar((char)line[i]);
  }
}
