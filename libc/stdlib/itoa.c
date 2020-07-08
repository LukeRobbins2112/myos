#include <stdlib.h>
#include <stdio.h>

static void reverse(char* str, int len){

  for (int i = 0; i < len/2; i++){
    char temp = str[i];
    str[i] = str[len - i - 1];
    str[len - i - 1] = temp;
  }
}

char* itoa(int value, char* str, int base){

  int i = 0;
  int isNegative = 0;
  
  // Handle zero explicitly
  if (value == 0) 
    { 
      str[i++] = '0'; 
      str[i] = '\0'; 
      return str; 
    } 

  // Handle negative numbers in base 10
  if (base == 10 && value < 0){
    value = -value;
    isNegative = 1;
  }

  // Perform the actual conversion
  while(value){
    int lowDigit = value % base;

    // For logDigit > 9, convert to hex equivalent
    str[i++] = (lowDigit > 9) ? ((lowDigit - 10) + 'A') : (lowDigit + '0');

    value /= base;
  }

  // Append sign if necessary
  if (isNegative){
    str[i++] = '-';
  }

  // Add null terminator
  str[i] = '\0';

  // Reverse string to get result
  // len is i, not (i+1), because we don't want to include null terminator
  reverse(str, i);

  return str;
}

char* utoa(uint32_t value, char* str, uint32_t base){

  int i = 0;
  
  // Handle zero explicitly
  if (value == 0) 
    { 
      str[i++] = '0'; 
      str[i] = '\0'; 
      return str; 
    } 

  // Perform the actual conversion
  while(value){
    uint32_t lowDigit = value % base;

    // For logDigit > 9, convert to hex equivalent
    str[i++] = (lowDigit > 9) ? ((lowDigit - 10) + 'A') : (lowDigit + '0');

    value /= base;
  }

  // Add null terminator
  str[i] = '\0';

  // Reverse string to get result
  // len is i, not (i+1), because we don't want to include null terminator
  reverse(str, i);

  return str;
}
