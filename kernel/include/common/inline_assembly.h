#ifndef _INLINE_ASSEMBLY_H
#define _INLINE_ASSEMBLY_H

#include <stdint.h>
#include <stdio.h>

static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

static inline void io_wait(void)
{
    asm volatile ( "jmp 1f\n\t"
                   "1:jmp 2f\n\t"
                   "2:" );
}

static inline void breakpoint(char* label)
{
  if (label){
    printf("Breakpoint %s\n", label);
  }
  
  asm volatile ( "xchgw %bx, %bx\n\t"
		 "xchgw %bx, %bx\n\t"
		 "xchgw %bx, %bx\n\t" );
}

static inline void CLI(void) {
  asm volatile("cli");
}

static inline void STI(void) {
  asm volatile("sti");
}

static inline void INT(uint8_t interrupt) {
  asm volatile("int %0"
	       : /* output */
	       : [interrupt] "g" (interrupt)
	       : /*clobbered*/);
}

#endif // _INLINE_ASSEMBLY_H
