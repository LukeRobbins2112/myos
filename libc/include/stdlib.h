#ifndef _STDLIB_H
#define _STDLIB_H 1
 
#include <sys/cdefs.h>
 
#ifdef __cplusplus
extern "C" {
#endif

// Inform the compiler that we won't return
// Aids optimization and prevents warnings
__attribute__((__noreturn__))
void abort(void);
 
#ifdef __cplusplus
}
#endif
 
#endif
