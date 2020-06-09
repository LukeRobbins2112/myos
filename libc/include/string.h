#ifndef _STRING_H
#define _STRING_H 1

#include <sys/cdefs.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// String and memory functions
// __restrict tells the compiler that ptr is the only way to access the object
// pointed by it and compiler doesn’t need to add any additional checks

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
size_t strlen(const char*);

  
#ifdef __cplusplus
}
#endif

#endif  // _STRING_H
