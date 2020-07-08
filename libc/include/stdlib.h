#ifndef _STDLIB_H
#define _STDLIB_H 1
 
#include <sys/cdefs.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Inform the compiler that we won't return
// Aids optimization and prevents warnings
__attribute__((__noreturn__))
void abort(void);

char* itoa(int value, char* str, int base);
char* utoa(uint32_t value, char* str, uint32_t base);  

  
#ifdef __cplusplus
}
#endif
 
#endif
