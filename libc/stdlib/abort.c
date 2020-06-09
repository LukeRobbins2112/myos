#include <stdio.h>
#include <stdlib.h>
 
__attribute__((__noreturn__))
void abort(void) {
#if defined(__is_libk)
  // If abort happens in kernel
  // TODO: Add proper kernel panic.
  printf("kernel: panic: abort()\n");
#else
  // If abort happens in userspace
  // TODO: Abnormally terminate the process as if by SIGABRT.
  printf("abort()\n");
#endif
  while (1) { }
  // Lets compiler know we'll never get here
  __builtin_unreachable();
}
